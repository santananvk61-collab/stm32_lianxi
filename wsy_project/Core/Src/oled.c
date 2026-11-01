#include "oled.h"
#include "main.h"
#include "i2c.h"

// OLED缓存区
static uint8_t oled_buffer[8 * 128] = {0};

// 画字起始坐标 (OLED_SetCursor设置, 画字函数从该位置往缓存区写字模)
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

// 发送命令:控制字节0x00表示后面是命令
// 返回HAL状态供上层检查;超时100ms,设备断线时不会永久卡死
static HAL_StatusTypeDef OLED_WriteCommand(uint8_t cmd)
{
    return HAL_I2C_Mem_Write(&hi2c1, OLED_I2C_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, &cmd, 1, 100);
}

DeviceError_t OLED_Init(void)
{
    HAL_Delay(100);//等待上电稳定

    uint8_t cmd [] =
    {
        0xAE, // 关闭显示
        0xD5, 0x80, // 设置时钟分频
        0xA8, 0x3F, // 设置多路复用率 (1/64)
        0xD3, 0x00, // 设置显示偏移
        0x40, // 设置显示起始行
        0x8D, 0x14, // 使能电荷泵 (关键!)
        0x20, 0x00, // 设置内存寻址模式 (水平地址模式)
        0xA1, // 设置段重映射 (左右翻转)
        0xC8, // 设置COM扫描方向 (上下翻转)
        0xDA, 0x12, // 设置COM引脚配置
        0x81, 0xCF, // 设置对比度
        0xD9, 0xF1, // 设置预充电周期
        0xDB, 0x40, // 设置VCOMH取消选择电平
        0x2E, // 关闭滚动
        0xA4, // 全局显示开启 (使用GDDRAM内容)
        0xA6, // 正常显示 (非反色)
        0xAF  // 开启显示
    };

    for (int i = 0; i < sizeof(cmd); i++)
    {
        if(OLED_WriteCommand(cmd[i]) != HAL_OK)
        {
            return ERR_NOT_RESPOND;   // 某条命令发不出去,设备可能没接/地址错
        }
    }

    OLED_Clear();
    return OLED_Update();   // 把空缓存区推到屏幕,同时返回Update的通信结果
}

void OLED_Clear(void)
{
    for (int i = 0; i < sizeof(oled_buffer); i++)
    {
        oled_buffer[i] = 0x00;
    }
}

// 设置字的起始坐标, 覆盖128x64范围 (x: 0~127, y: 0~63)
// 坐标超范围不裁减, 自动回绕: x按128列取余, y按64行取余
void OLED_SetCursor(uint8_t x, uint8_t y)
{
    cursor_x = x % 128;
    cursor_y = y % 64;
}

// 换行: 回到行首并下移一行(12像素), 超过屏幕底部回到第一行
static void OLED_LineFeed(void)
{
    cursor_x = 0;
    cursor_y += 14;
    if (cursor_y + 14 > 64)
    {
        cursor_y = 0;
    }
}

// 在缓存区当前坐标画一个字模 (cols列x12行), 逐列2字节低位在上, 按位或写入
// y不对齐页边界时自动移位, 最多跨3个页
static void OLED_DrawGlyph(const uint8_t *glyph, uint8_t cols)
{
    uint8_t  page  = cursor_y / 8;
    uint8_t  shift = cursor_y % 8;
    uint16_t base  = page * 128 + cursor_x;

    for (uint8_t i = 0; i < cols; i++)
    {
        uint32_t bits = ((uint32_t)glyph[2 * i] |
                         ((uint32_t)(glyph[2 * i + 1] & 0x0F) << 8)) << shift;
        oled_buffer[base + i] |= (uint8_t)(bits & 0xFF);
        if (page < 7)
        {
            oled_buffer[base + i + 128] |= (uint8_t)((bits >> 8) & 0xFF);
        }
        if (page < 6)
        {
            oled_buffer[base + i + 256] |= (uint8_t)((bits >> 16) & 0x07);
        }
    }
}

// 显示一个ASCII字符 (6x12), 超出右边界自动换行
void OLED_ShowChar(char c)
{
    if (c < 0x20 || c > 0x7E)
    {
        c = '?';
    }
    if (cursor_x + FONT_ASCII_W > 128)
    {
        OLED_LineFeed();
    }
    OLED_DrawGlyph(font_ascii[c - 0x20], FONT_ASCII_W);
    cursor_x += FONT_ASCII_W;
}

// 显示汉字字库中第index个汉字 (12x12), 索引表见font.h, 超出右边界自动换行
void OLED_ShowCN(uint8_t index)
{
    if (index >= FONT_CN_NUM)
    {
        return;
    }
    if (cursor_x + FONT_CN_W > 128)
    {
        OLED_LineFeed();
    }
    OLED_DrawGlyph(font_cn[index], FONT_CN_W);
    cursor_x += FONT_CN_W;
}

// 显示字符串 (汉字/英文/数字/特殊符号混合), 自动识别UTF-8汉字, 自动换行
void OLED_ShowString(const char *s)
{
    while (*s)
    {
        if ((uint8_t)*s < 0x80)
        {
            // ASCII字符 (单字节)
            OLED_ShowChar(*s++);
        }
        else
        {
            // UTF-8汉字 (3字节), 在cn_map中查找索引
            uint8_t found = 0;
            for (uint8_t i = 0; i < FONT_CN_NUM; i++)
            {
                if (s[0] == cn_map[i * 3] &&
                    s[1] == cn_map[i * 3 + 1] &&
                    s[2] == cn_map[i * 3 + 2])
                {
                    OLED_ShowCN(i);
                    found = 1;
                    break;
                }
            }
            s += 3;  // 无论是否找到都跳过3字节
            if (!found)
            {
                // 字库中没有的字, 显示问号占位
                OLED_ShowChar('?');
            }
        }
    }
}

// 把缓存区的数据传到屏幕上 (整屏刷新, 不需要指定大小)
// 第一步设置显示窗口(列0~127,页0~7),第二步写1024字节显存
// 任一步I2C失败返回 ERR_NOT_RESPOND,超时100ms不卡死
DeviceError_t OLED_Update(void)
{
    uint8_t cmds[6] = {0x21, 0, 127, 0x22, 0, 7};
    if(HAL_I2C_Mem_Write(&hi2c1, OLED_I2C_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, cmds, 6, 100) != HAL_OK)
    {
        return ERR_NOT_RESPOND;
    }
    if(HAL_I2C_Mem_Write(&hi2c1, OLED_I2C_ADDR, 0x40, I2C_MEMADD_SIZE_8BIT, oled_buffer, sizeof(oled_buffer), 100) != HAL_OK)
    {
        return ERR_NOT_RESPOND;
    }
    return ERR_OK;
}