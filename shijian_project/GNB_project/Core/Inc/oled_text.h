#ifndef INC_OLED_TEXT_H_
#define INC_OLED_TEXT_H_

#include "main.h"   /* uint8_t 等基础类型 */

/* =====================================================================
 * OLED 驱动 · 文字模块（oled_text.h）
 * ---------------------------------------------------------------------
 * 只管文字排版：光标、字模绘制、英文/中文/UTF-8 混排。
 * 依赖 oled.c 的 DrawPixel 和 font.c 的字模数据，实现在 oled_text.c。
 * 正常情况下不需要单独 include 本文件——oled.h 已经把它 include 进来了。
 * ===================================================================== */

/* 设置光标：下一个字符画在哪个像素位置 */
void SSD1306_SetCursor(uint8_t x, uint8_t y);

/* 画一个字模：在 (x,y) 画宽 w、高 h 的字模。
 *   汉字/符号传 (12, 12)，英文数字传 (6, 12)。
 *   例：SSD1306_DrawGlyph(0, 0, FontCN12x12[0], 12, 12);   // 左上角画"波" */
void SSD1306_DrawGlyph(uint8_t x, uint8_t y, const uint8_t *glyph, uint8_t w, uint8_t h);

/* 英文 / 数字 */
void SSD1306_WriteChar(char ch);            /* 写一个字符（宽 6 高 12） */
void SSD1306_WriteString(const char *str);  /* 写一串字符 */
void SSD1306_WriteInt(int val);             /* 写一个整数（支持负数） */

/* 中文 / 符号 */
void SSD1306_WriteCN(uint8_t index);        /* 在光标处显示汉字表第 index 个字（0=波 4=温 5=度） */
void SSD1306_WriteCNString(const char *str);/* 显示 UTF-8 字符串，汉字、符号、英文可混写 */

#endif /* INC_OLED_TEXT_H_ */
