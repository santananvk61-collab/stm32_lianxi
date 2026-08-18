/*
 * ssd1306.h —— 0.96 寸 SSD1306 OLED（I2C 接口）驱动头文件
 *
 * 作用：把 "在屏幕上画点 / 写字符" 这种高级操作，翻译成 I2C 总线上的
 *       字节序列，发给 OLED 模块。上层（main.c）只需要调用 Init / Clear /
 *       WriteString 等函数，不用关心 I2C 细节。
 */
#ifndef __SSD1306_H
#define __SSD1306_H

#include "main.h"
#include "stm32f1xx_hal.h"

/* OLED 的 I2C 从机地址：7 位地址是 0x3C，HAL 要求左移 1 位变成写地址 0x78。
 * 如果你的屏点不亮，把这里的 0x3C 改成 0x3D 再试（少数模块用 0x3D）。 */
#define SSD1306_I2C_ADDR    (0x3C << 1)

#define SSD1306_WIDTH       128   /* 屏幕宽：128 像素 */
#define SSD1306_HEIGHT      64    /* 屏幕高：128×64 屏填 64；如果是 128×32 屏改成 32 */
#define SSD1306_BUF_SIZE    (SSD1306_WIDTH * SSD1306_HEIGHT / 8)  /* 显存大小：1024 字节 */

/* I2C 控制字节：紧跟它的数据，是"命令"还是"显存像素" */
#define SSD1306_CMD     0x00   /* 接下来的字节是命令（配置屏） */
#define SSD1306_DATA    0x40   /* 接下来的字节是显存数据（要显示的内容） */

/* 对外提供的函数（main.c 里调用） */
void SSD1306_Init(void);                       /* 上电初始化 OLED */
void SSD1306_Clear(void);                      /* 清屏（全黑） */
void SSD1306_UpdateScreen(void);               /* 把显存刷到屏幕 */
void SSD1306_SetCursor(uint8_t x, uint8_t y);  /* 设置下一个字符的位置（像素坐标） */
void SSD1306_WriteChar(char ch);               /* 写一个字符 */
void SSD1306_WriteString(const char *str);     /* 写一串字符 */
void SSD1306_WriteInt(int val);                /* 写一个整数（支持负数） */

#endif /* __SSD1306_H */
