/**
  * @file    bsp_at8236.c
  * @brief   双 AT8236 H 桥驱动
  * @note   通道映射见 bsp_at8236.h；方向符号在此统一修正。
  */

#include "bsp_at8236.h"
#include "tim.h"

/* 按真值表写入一路 H 桥：正转 IN1=PWM/IN2=0，反转反之，0 为滑行 */
static void motor_write(uint32_t ch_in1, uint32_t ch_in2, int16_t cmd, int8_t sign)
{
    int16_t d = (int16_t)(cmd * sign);

    if (d > MOTOR_PWM_MAX) {
        d = MOTOR_PWM_MAX;
    } else if (d < -MOTOR_PWM_MAX) {
        d = -MOTOR_PWM_MAX;
    }

    if (d > 0) {
        __HAL_TIM_SET_COMPARE(&htim4, ch_in1, (uint16_t)d);
        __HAL_TIM_SET_COMPARE(&htim4, ch_in2, 0u);
    } else if (d < 0) {
        __HAL_TIM_SET_COMPARE(&htim4, ch_in1, 0u);
        __HAL_TIM_SET_COMPARE(&htim4, ch_in2, (uint16_t)(-d));
    } else {
        __HAL_TIM_SET_COMPARE(&htim4, ch_in1, 0u);
        __HAL_TIM_SET_COMPARE(&htim4, ch_in2, 0u);
    }
}

void Motor_Init(void)
{
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0u);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0u);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0u);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, 0u);

    (void)HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
    (void)HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
    (void)HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
    (void)HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
}

void Motor_Set(int16_t pwm_l, int16_t pwm_r)
{
    motor_write(TIM_CHANNEL_4, TIM_CHANNEL_3, pwm_l, MOT_L_SIGN);
    motor_write(TIM_CHANNEL_2, TIM_CHANNEL_1, pwm_r, MOT_R_SIGN);
}

void Motor_Coast(void)
{
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0u);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0u);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0u);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, 0u);
}

void Motor_Brake(void)
{
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, (uint32_t)MOTOR_PWM_MAX);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, (uint32_t)MOTOR_PWM_MAX);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, (uint32_t)MOTOR_PWM_MAX);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, (uint32_t)MOTOR_PWM_MAX);
}
