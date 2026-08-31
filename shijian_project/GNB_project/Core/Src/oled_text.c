/* =====================================================================
 * OLED 驱动 · 文字模块（oled_text.c）
 * ---------------------------------------------------------------------
 * 职责：把文字排到屏幕上 —— 管光标、画字模、自动换行。
 *
 * 依赖两个东西：
 *   oled.c 的 SSD1306_DrawPixel()  —— 负责"点亮某个像素"
 *   font.c 的字模数据              —— 负责"某个字长什么样"
 * 本文件只管中间这层：拿到字模，按光标位置一个个点亮像素，并推进光标。
 *
 * 三个模块的分工：
 *   font.c      -> 字模数据（纯数据）
 *   oled.c      -> 画点/画线（显存 + 图元）
 *   oled_text.c -> 文字排版（本文件：光标 + 字模调用）
 * ===================================================================== */

#include "oled.h"   // 拿 SSD1306_DrawPixel、SSD1306_WIDTH 等
#include "font.h"   // 拿 FontCN12x12 等字模数据、FONT_* 常量

/* ==================== 光标 ====================
 * 光标 = "下一个字画在哪个像素位置"。所有 Write* 函数都从光标处开始画，
 * 画完自动把光标往右推，推到右边界就换行。 */
static uint8_t CursorX = 0;
static uint8_t CursorY = 0;

/* 设置光标（像素坐标）——想在屏幕哪个位置开始写字，先调它 */
void SSD1306_SetCursor(uint8_t x, uint8_t y)
{
    CursorX = x;
    CursorY = y;
}

/* 内部小工具：光标往右推 w 像素，推到右边界就换行。
 * 三个写字符的函数都要做这件事，抽出来避免重复。 */
static void AdvanceCursor(uint8_t w)
{
    CursorX += w;
    if (CursorX > SSD1306_WIDTH - w)
    {
        CursorX = 0;
        CursorY += FONT_LINE_PITCH;   /* 行距 16，比字高 12 多 4 像素，行间不糊 */
    }
}

/* ==================== 字模绘制 ====================
 * 本文件唯一的"画字"函数，汉字、符号、英文数字都走它，只是宽度不同：
 *   汉字/符号：宽 12、高 12 -> SSD1306_DrawGlyph(x, y, g, 12, 12)
 *   英文数字：宽  6、高 12 -> SSD1306_DrawGlyph(x, y, g,  6, 12)
 *
 * 字模在内存里的排布（font.h 有详细说明）：
 *   按"块"存，每块 8 行（正好是 OLED 的一页），块内逐列存：
 *     第 n 字节 = 第 n 列，bit0 = 这一块最上面一行
 *   高 12 = 2 块：块 0 管第 0~7 行，块 1 管第 8~15 行（只用前 4 行） */
void SSD1306_DrawGlyph(uint8_t x, uint8_t y, const uint8_t *glyph, uint8_t w, uint8_t h)
{
    uint8_t blocks = (h + 7) / 8;      /* 12 高 = 2 块 */

    for (uint8_t blk = 0; blk < blocks; blk++)
    {
        for (uint8_t col = 0; col < w; col++)
        {
            uint8_t bits = glyph[blk * w + col];   /* 这一列在这一块里的 8 个点 */
            for (uint8_t row = 0; row < 8; row++)
            {
                uint8_t yy = blk * 8 + row;        /* 换算成字模里的行号 */
                if (yy < h && (bits & (1 << row))) /* 超出字模高度的行不画 */
                    SSD1306_DrawPixel(x + col, y + yy, 1);
            }
        }
    }
}

/* ==================== 英文 / 数字 ==================== */

/* 写一个英文字符/数字（宽 6 高 12，画完光标右移 6） */
void SSD1306_WriteChar(char ch)
{
    /* 字模表只覆盖空格(0x20)到波浪号(0x7E)，其它字符（如回车）当空格处理 */
    if (ch < 0x20 || ch > 0x7E) ch = 0x20;

    SSD1306_DrawGlyph(CursorX, CursorY,
                      FontASCII6x12[ch - 0x20], FONT_ASCII_WIDTH, FONT_HEIGHT);
    AdvanceCursor(FONT_ASCII_WIDTH);
}

/* 写一串英文字符/数字 */
void SSD1306_WriteString(const char *str)
{
    while (*str)
        SSD1306_WriteChar(*str++);
}

/* 写一个整数（自己拆位，不依赖 printf 库）。
 * 支持负数；用 12 字节缓冲区兜底（32 位整数最多 10 位 + 负号 + 结尾 \0）。 */
void SSD1306_WriteInt(int val)
{
    char buf[12];
    uint32_t u;
    uint8_t i = 0;

    if (val < 0)
    {
        SSD1306_WriteChar('-');
        u = (uint32_t)(-(val + 1)) + 1;  /* 绕开 INT_MIN 取反会溢出的坑 */
    }
    else
    {
        u = (uint32_t)val;
    }

    /* 从个位往上拆，先存成"倒序"的字符 */
    do
    {
        buf[i++] = (char)('0' + (u % 10));
        u /= 10;
    } while (u > 0);

    /* 倒序存进去的，现在反过来输出就是正确的顺序 */
    while (i > 0)
        SSD1306_WriteChar(buf[--i]);
}

/* ==================== 中文 / 符号 ==================== */

/* 在当前光标处显示汉字表第 index 个字，画完光标右移 12。
 * 例：SSD1306_SetCursor(0, 0); SSD1306_WriteCN(0);   // 左上角显示"波" */
void SSD1306_WriteCN(uint8_t index)
{
    if (index >= CN_FontCount) return;   /* 下标越界，直接不画 */
    SSD1306_DrawGlyph(CursorX, CursorY, FontCN12x12[index], FONT_CN_WIDTH, FONT_HEIGHT);
    AdvanceCursor(FONT_CN_WIDTH);
}

/* 内部小工具：解码 UTF-8 的一个字符，返回它的 Unicode 编码。
 * 参数 p：指向"字符串指针"的地址，读完会把指针推到下一个字符。
 * 原理（不用背，知道"中文占 3 字节"就行）：
 *   UTF-8 英文 = 1 字节 0xxxxxxx
 *   常见符号    = 2 字节 110xxxxx 10xxxxxx
 *   中文        = 3 字节 1110xxxx 10xxxxxx 10xxxxxx
 * 把那些 x 位按规则拼起来就是 Unicode 编码。 */
static uint16_t DecodeUTF8(const char **p)
{
    uint8_t c = (uint8_t)**p;
    uint16_t unicode;

    if ((c & 0x80) == 0x00)          /* 0xxxxxxx：英文/半角，1 字节 */
    {
        unicode = c;
        (*p) += 1;
    }
    else if ((c & 0xE0) == 0xC0)     /* 110xxxxx：2 字节字符 */
    {
        unicode = (uint16_t)((c & 0x1F) << 6);
        (*p) += 1;
        unicode |= (uint8_t)(**p & 0x3F);
        (*p) += 1;
    }
    else                              /* 1110xxxx：中文，3 字节 */
    {
        unicode = (uint16_t)((c & 0x0F) << 12);
        (*p) += 1;
        unicode |= (uint16_t)((**p & 0x3F) << 6);
        (*p) += 1;
        unicode |= (uint8_t)(**p & 0x3F);
        (*p) += 1;
    }
    return unicode;
}

/* 显示 UTF-8 字符串，汉字、英文数字、符号可以随便混着写。
 *
 * 每个字符的处理：
 *   英文/数字（<0x80） -> 画 6 宽 12 高（正好是汉字的一半宽）
 *   汉字/符号（>=0x80）-> 解码出 Unicode 编码，先查汉字表再查符号表，
 *                        找到就画 12 宽 12 高，两张表都没有就空一格跳过
 *
 * 用法示例：
 *   SSD1306_SetCursor(0, 16);
 *   SSD1306_WriteCNString("温度:25.3℃");
 *   SSD1306_WriteCNString("气压:1013hPa");
 * 纯英文/数字用 SSD1306_WriteString() 也行，效果一样。 */
void SSD1306_WriteCNString(const char *str)
{
    while (*str)
    {
        if ((uint8_t)*str < 0x80)        /* 英文/数字/半角符号 */
        {
            SSD1306_WriteChar(*str);     /* 复用：画 6x12、推光标、判断换行 */
            str++;
            continue;
        }

        /* 汉字或符号：先解码出 Unicode，再查表 */
        uint16_t unicode = DecodeUTF8(&str);
        uint8_t found = 0;

        for (uint8_t i = 0; i < CN_FontCount; i++)     /* 先查汉字表 */
        {
            if (CN_Unicode[i] == unicode)
            {
                SSD1306_DrawGlyph(CursorX, CursorY, FontCN12x12[i], FONT_CN_WIDTH, FONT_HEIGHT);
                found = 1;
                break;
            }
        }
        if (found == 0)                                /* 汉字表没有，再查符号表 */
        {
            for (uint8_t i = 0; i < SYM_FontCount; i++)
            {
                if (SYM_Unicode[i] == unicode)
                {
                    SSD1306_DrawGlyph(CursorX, CursorY, FontSYM12x12[i], FONT_CN_WIDTH, FONT_HEIGHT);
                    found = 1;
                    break;
                }
            }
        }

        AdvanceCursor(FONT_CN_WIDTH);    /* 找到没找到都占一个汉字的宽度 */
    }
}
