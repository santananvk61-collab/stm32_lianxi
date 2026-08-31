/* =====================================================================
 * OLED 驱动 · 核心模块（oled.c）
 * ---------------------------------------------------------------------
 * 职责：只干三件事 ——
 *   1. 底层：通过 I2C 给 SSD1306 发命令 / 发数据
 *   2. 显存：管理 1024 字节的显示缓冲区（先画进缓冲区，再整体刷屏）
 *   3. 图元：画点、画线、清屏、刷屏、上电初始化
 *
 * 本文件【不包含任何文字显示逻辑】。文字（光标、字符、中文、UTF-8）
 * 全部拆到 oled_text.c 里了，字模数据在 font.c 里。
 * 想画字，看 oled_text.c；想加新汉字，看 font.c。
 *
 * 三个模块的分工：
 *   font.c      -> 回答"某个字长什么样"（纯数据）
 *   oled.c      -> 回答"怎么画点/线"（显存 + 图元）
 *   oled_text.c -> 回答"怎么把文字排到屏幕上"（光标 + 字模调用）
 * ===================================================================== */

#include "oled.h"

/* ==================== 显存 ====================
 * 1024 字节，存在 RAM 里。屏幕上的每个像素对应显存里的一个 bit：
 *   第 (y/8) 页、第 x 列的那个字节，它的第 (y%8) 位就是 (x, y) 这个点。
 * 为什么先画进显存、最后才刷屏？因为 I2C 一次发一大块比一个个点发快得多，
 * 而且所有绘图操作只改内存，最后统一上屏，不会闪烁。 */
static uint8_t SSD1306_Buffer[SSD1306_BUF_SIZE];

/* ==================== 底层 I2C 发送 ====================
 * 用 HAL 的"存储器写"发数据，它发出的格式正好是
 * [从机地址][控制字节][数据...]，和 SSD1306 协议要求的完全一致。
 * 控制字节决定后面的数据是"命令"还是"显存内容"：0x00=命令，0x40=数据。
 *
 * 关于严谨性：这两个函数不带返回值（显示是"尽力而为"——这一帧没刷上，
 * 下一帧还会再刷，不影响程序继续跑，所以没必要像传感器那样每一步都检查）。
 * 但带了 100ms 超时，I2C 卡住时最多等 100ms 就返回，绝不会死等。 */

/* 发一个数据包：既可以发命令，也可以发显存数据（由 ctrl 区分） */
static void SSD1306_WriteI2C(uint8_t ctrl, uint8_t *data, uint16_t size)
{
    HAL_I2C_Mem_Write(&hi2c1, SSD1306_I2C_ADDR, ctrl, 1, data, size, 100);
}

/* 发一条命令（用于配置屏幕） */
static void SSD1306_WriteCmd(uint8_t cmd)
{
    SSD1306_WriteI2C(SSD1306_CMD, &cmd, 1);
}

/* ==================== 上电初始化 ====================
 * 这一串命令是 SSD1306 数据手册规定的上电序列，0.96 寸模块基本通用。
 * 每条命令具体干嘛看注释，看不懂也没关系，照发就行。 */
void SSD1306_Init(void)
{
    uint8_t init_cmds[] = {
        0xAE,             // 关闭显示（先关着，配完再开，避免花屏）
        0xD5, 0x80,       // 设置时钟分频
        0xD3, 0x00,       // 显示偏移 0
        0x40,             // 起始行 0
        0x8D, 0x14,       // 开启电荷泵（OLED 需要升压才能亮）
        0x20, 0x00,       // 显存寻址模式：水平
        0xA1,             // 段重映射（左右方向，适配常见模块）
        0xC8,             // COM 扫描方向（上下方向）
        0x81, 0x7F,       // 对比度（0x7F，太大会过亮发虚）
        0xD9, 0xF1,       // 预充电周期
        0xDB, 0x40,       // VCOMH 电平
        0xA4,             // 显示内容跟随显存
        0xA6              // 正常显示（非反色）
    };
    for (uint8_t i = 0; i < sizeof(init_cmds); i++)
        SSD1306_WriteCmd(init_cmds[i]);

    /* 根据屏幕高度发"行数"和"引脚"配置：
     *   128×64 屏：0xA8 0x3F / 0xDA 0x12
     *   128×32 屏：0xA8 0x1F / 0xDA 0x02 */
    if (SSD1306_HEIGHT == 64)
    {
        SSD1306_WriteCmd(0xA8); SSD1306_WriteCmd(0x3F);
        SSD1306_WriteCmd(0xDA); SSD1306_WriteCmd(0x12);
    }
    else
    {
        SSD1306_WriteCmd(0xA8); SSD1306_WriteCmd(0x1F);
        SSD1306_WriteCmd(0xDA); SSD1306_WriteCmd(0x02);
    }

    SSD1306_Clear();
    SSD1306_UpdateScreen();

    SSD1306_WriteCmd(0xAF);   // 打开显示
}

/* ==================== 显存操作 ==================== */

/* 清屏：显存全部写 0（全黑） */
void SSD1306_Clear(void)
{
    for (uint16_t i = 0; i < SSD1306_BUF_SIZE; i++)
        SSD1306_Buffer[i] = 0x00;
}

/* 刷屏：把整块 1024 字节显存通过 I2C 发给 OLED */
void SSD1306_UpdateScreen(void)
{
    /* 先告诉 OLED 数据要写到哪：从第 0 列第 0 页开始，铺满全屏 */
    SSD1306_WriteCmd(0x21);             // 设置列地址
    SSD1306_WriteCmd(0);                // 起始列
    SSD1306_WriteCmd(127);              // 结束列
    SSD1306_WriteCmd(0x22);             // 设置页地址
    SSD1306_WriteCmd(0);                // 起始页
    SSD1306_WriteCmd((SSD1306_HEIGHT / 8) - 1);  // 结束页（64高=7，32高=3）

    /* 再一次性把 1024 字节显存当"数据"发出去 */
    SSD1306_WriteI2C(SSD1306_DATA, SSD1306_Buffer, SSD1306_BUF_SIZE);
}

/* ==================== 基本图元 ==================== */

/* 画一个像素：on=1 点亮，on=0 熄灭。
 * 显存里 (x, y) 对应的 bit 是 第(y/8)页 第 x 列 的第(y%8)位。 */
void SSD1306_DrawPixel(uint8_t x, uint8_t y, uint8_t on)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;  // 越界忽略

    if (on)
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |=  (1 << (y % 8));
    else
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
}

/* 用 Bresenham 算法画任意直线（含斜线）。
 * 关键好处：x 每走一格就至少打一个点，所以每一列都有像素，
 * 斜线画出来是连续的，不会断成虚线。
 * 例：SSD1306_DrawLine(0, 0, 127, 63, 1);  画左上到右下的整条对角线 */
void SSD1306_DrawLine(int x0, int y0, int x1, int y1, uint8_t on)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int sx = (dx > 0) ? 1 : -1;   // x 方向步长（+1 右 / -1 左）
    int sy = (dy > 0) ? 1 : -1;   // y 方向步长（+1 下 / -1 上）
    dx = (dx > 0) ? dx : -dx;     // 取绝对值，后面只比较大小
    dy = (dy > 0) ? dy : -dy;

    int err = dx - dy;            // 误差项，决定下一步走 x 还是 y
    while (1)
    {
        SSD1306_DrawPixel((uint8_t)x0, (uint8_t)y0, on);
        if (x0 == x1 && y0 == y1) break;   // 到终点就停
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}
