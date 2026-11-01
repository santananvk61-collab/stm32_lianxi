/**
  * @file    bsp_at8236.h
  * @brief   双 AT8236 H 桥（TIM4 四路 PWM，20kHz）
  * @note   IN1/IN2：00 滑行、10 正转、01 反转、11 制动。
  *         左轮 U5：IN1=PB9(CH4)，IN2=PB8(CH3)；右轮 U6：IN1=PB7(CH2)，IN2=PB6(CH1)。
  *         S1 方向标定只改 MOT_L_SIGN / MOT_R_SIGN。
  */

#ifndef __BSP_AT8236_H
#define __BSP_AT8236_H

#include <stdint.h>

#define MOTOR_PWM_MAX   4199     /* = TIM4 ARR，20kHz */

#define MOT_L_SIGN      (+1)
#define MOT_R_SIGN      (+1)

void Motor_Init(void);
void Motor_Set(int16_t pwm_l, int16_t pwm_r);  /* 有符号占空比 ±MOTOR_PWM_MAX */
void Motor_Coast(void);
void Motor_Brake(void);

#endif /* __BSP_AT8236_H */
