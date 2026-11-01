/**
  * @file    bsp_encoder.h
  * @brief   TIM3/TIM2 正交编码器（TI12 四倍频）
  * @note   每个 5ms 控制周期读一次并清零；该窗口内计数远小于 32768，不会溢出。
  */

#ifndef __BSP_ENCODER_H
#define __BSP_ENCODER_H

#include <stdint.h>

/* S1 方向标定：车向前推时计数应为正，反了改 -1 */
#define ENC_L_SIGN   (+1)   /* 左轮：TIM3 (PB4/PB5) */
#define ENC_R_SIGN   (+1)   /* 右轮：TIM2 (PA15/PB3) */

void    Encoder_Init(void);
int16_t Encoder_ReadLeft(void);    /* 返回上次读取后的增量并清零 */
int16_t Encoder_ReadRight(void);
void    Encoder_Reset(void);

#endif /* __BSP_ENCODER_H */
