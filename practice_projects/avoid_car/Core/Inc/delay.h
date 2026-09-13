#ifndef __DELAY_H
#define __DELAY_H

#include "main.h"

/* 用 DWT 计数器做微秒级延时（比 HAL_Delay 精确，HAL_Delay 只能到 ms） */

void Delay_Init(void);
void delay_us(uint32_t us);

#endif
