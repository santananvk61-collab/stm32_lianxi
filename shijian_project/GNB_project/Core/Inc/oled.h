#ifndef INC_OLED_H_
#define INC_OLED_H_

#include "i2c.h"
#include "main.h"
#include "stm32f1xx_hal.h"
#include "font.h"   /* 字模数据在 font.c，字号/行距常量在 font.h */

/* =====================================================================
 * OLED 驱动 · 总入口头文件（oled.h）
 * ---------------------------------------------------------------------
 * 只声明"核心模块"（显存 + 图元）的函数。
 * 文字模块在单独的 oled_text.h / oled_text.c 里，本文件末尾把它
 * include 进来，这样 main.c 只要 include "oled.h" 一个文件，
 * 核心和文字的所有函数就都能用了。
 *
 * 三个模块的分工：
 *   font.c      -> 字模数据（纯数据）
 *   oled.c      -> 画点/画线（显存 + 图元）
 *   oled_text.c -> 文字排版（光标 + 字模调用）
 * ===================================================================== */

/* 屏幕硬件参数 */
#define SSD1306_I2C_ADDR    (0x3C << 1)   /* 从机地址：7 位 0x3C 左移 1 位。
                                             点不亮就试 0x3D（少数模块用它） */
#define SSD1306_WIDTH       128           /* 屏宽：128 像素 */
#define SSD1306_HEIGHT      64            /* 屏高：128×64 填 64；128×32 屏改成 32 */
#define SSD1306_BUF_SIZE    (SSD1306_WIDTH * SSD1306_HEIGHT / 8)  /* 显存：1024 字节 */

/* I2C 控制字节：决定后面的数据是"命令"还是"显存内容" */
#define SSD1306_CMD     0x00   /* 命令（配置屏幕） */
#define SSD1306_DATA    0x40   /* 数据（要显示的像素） */

/* ---- 核心模块：显存 + 图元（实现在 oled.c） ---- */
void SSD1306_Init(void);                                               /* 上电初始化 */
void SSD1306_Clear(void);                                              /* 清屏（全黑） */
void SSD1306_UpdateScreen(void);                                       /* 把显存刷到屏幕 */
void SSD1306_DrawPixel(uint8_t x, uint8_t y, uint8_t on);              /* 画一个点（on=1亮/0灭） */
void SSD1306_DrawLine(int x0, int y0, int x1, int y1, uint8_t on);     /* 画直线（含斜线，连续不虚） */

/* ---- 文字模块（实现在 oled_text.c，这里汇总声明） ---- */
#include "oled_text.h"

#endif /* INC_OLED_H_ */
