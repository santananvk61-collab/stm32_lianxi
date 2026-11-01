#ifndef __OLED_H__

#define __OLED_H__

#include <stdint.h>
#include "font.h"
#include "error_code.h"

#define  OLED_I2C_ADDR  (0x3c << 1)             // OLED I2C地址(括号不能省,防止宏展开后优先级出错)

DeviceError_t OLED_Init(void);                  // 初始化OLED,失败返回 ERR_NOT_RESPOND
void OLED_Clear(void);                          // 清除OLED屏幕(只清缓存区,无硬件操作)
void OLED_SetCursor(uint8_t x, uint8_t y);      // 设置字的起始坐标 (x: 0~127, y: 0~63, 超范围自动回绕)
DeviceError_t OLED_Update(void);                // 把缓存区数据整屏传到屏幕 (128x64),失败返回 ERR_NOT_RESPOND
void OLED_ShowChar(char c);                     // 显示一个ASCII字符 (6x12)
void OLED_ShowString(const char *s);            // 显示字符串 (汉字/英文/数字/特殊符号混合), 自动识别UTF-8汉字, 自动换行
void OLED_ShowCN(uint8_t index);                // 显示汉字字库第index个汉字 (12x12, 索引表见font.h)

#endif
