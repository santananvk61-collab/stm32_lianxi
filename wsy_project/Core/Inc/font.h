#ifndef __FONT_H__

#define __FONT_H__

#include <stdint.h>

/*
 * 字模格式 (PCtoLCD2002设置: 阴码/逐列式/逆向(低位在前)/十六进制):
 *   每列2字节: 第1字节 = 该列第0~7行 (bit0为最上行)
 *              第2字节 = 该列第8~11行 (bit0为第8行, 高4位为0)
 *
 * 想新增汉字: 用PCtoLCD2002按上面设置取12x12模, 在font.c的font_cn数组末尾
 *   按同样格式添加一行, 并把FONT_CN_NUM加1、同步更新下面的索引表注释
 */
#define FONT_ASCII_W    6       // ASCII字符宽度(列)
#define FONT_CN_W       12      // 汉字宽度(列)
#define FONT_H          12      // 字高(行)

#define FONT_CN_NUM     50      // 汉字个数

extern const uint8_t font_ascii[95][12];        // ASCII字库, 下标 = 字符 - 0x20
extern const uint8_t font_cn[FONT_CN_NUM][24];  // 汉字字库
extern const char cn_map[];                     // 汉字UTF-8索引表 (OLED_ShowString用)

/*
 * 汉字索引表 (OLED_ShowCN的index):
 *  0:温  1:度  2:湿  3:时  4:间  5:日  6:期  7:年  8:月  9:周
 * 10:一 11:二 12:三 13:四 14:五 15:六 16:分 17:秒 18:℃ 19:正
 * 20:负 21:上 22:下 23:限 24:报 25:警 26:开 27:关 28:启 29:动
 * 30:停 31:止 32:设 33:置 34:模 35:式 36:状 37:态 38:成 39:功
 * 40:失 41:速 42:电 43:流 44:压 45:系 46:统 47:运 48:行
 * 49:气
 */

#endif