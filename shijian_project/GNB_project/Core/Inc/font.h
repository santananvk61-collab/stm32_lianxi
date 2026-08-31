#ifndef INC_FONT_H_
#define INC_FONT_H_

#include "main.h"

/* =====================================================================
 * 字库头文件 font.h —— 只有 12x12 一种字号
 * ---------------------------------------------------------------------
 * 三张字模表（数据全在 font.c）：
 *   FontASCII6x12 —— 英文/数字/半角符号，宽 6、高 12，共 95 个
 *   FontCN12x12   —— 汉字，宽 12、高 12，共 9 个（波特律动温度气压湿）
 *   FontSYM12x12  —— 特殊符号，宽 12、高 12，共 1 个（℃）
 *
 * 取模格式（三张表统一）：
 *   按"块"存，每块 8 行（正好是 OLED 的一页），块内逐列存：
 *     第 n 字节 = 第 n 列，字节的 bit0 = 这一块的最上面一行
 *   高 12 = 2 块：前一半字节管第 0~7 行，后一半字节管第 8~15 行（只用前 4 行）
 *
 * 排版建议（128x64 的屏）：
 *   行距用 16，字号 12，行间自然空出 5 像素，不会糊在一起。
 *   一屏 5 行；每行 10 个汉字，或 21 个英文数字。
 * ===================================================================== */

/* 行距：比字高多 4 像素。12x12 字模的墨迹占满第 0~11 行，
 * 配 16 像素行距时，上一行最后一个墨点到下一行第一个墨点正好空 4 像素，
 * 不会糊在一起。三行数据占 48 像素，128x64 的屏一屏能放 5 行。
 * 想改行距只改这一个数（建议 14~18，小于 12 就会粘连）。 */
#define FONT_LINE_PITCH     16

/* 字模尺寸 */
#define FONT_HEIGHT         12   /* 字高：12 像素 */
#define FONT_ASCII_WIDTH    6    /* 英文数字宽：6 像素（正好是汉字的一半） */
#define FONT_CN_WIDTH       12   /* 汉字/符号宽：12 像素 */
#define FONT_ASCII_BYTES    12   /* 一个英文数字占 12 字节 */
#define FONT_CN_BYTES       24   /* 一个汉字占 24 字节 */

/* 半角 ASCII 字库：FontASCII6x12['A' - 0x20] 就是字母 A 的字模 */
extern const uint8_t  FontASCII6x12[][12];

/* 汉字字库 */
extern const uint8_t  FontCN12x12[][24];
extern const uint16_t CN_Unicode[];    /* CN_Unicode[i] = FontCN12x12[i] 是哪个字 */

/* 特殊符号字库（目前只有 ℃） */
extern const uint8_t  FontSYM12x12[][24];
extern const uint16_t SYM_Unicode[];

/* 两张表的当前字数（font.c 里用 sizeof 自动统计，加字后不用改） */
extern const uint8_t  CN_FontCount;
extern const uint8_t  SYM_FontCount;

#endif /* INC_FONT_H_ */
