/**
  * @file    bsp_encoder.c
  * @brief   TIM3/TIM2 正交编码器驱动
  * @note   左轮 TIM3(PB4/PB5)，右轮 TIM2(PA15/PB3)。
  */

#include "bsp_encoder.h"
#include "tim.h"

/* 读计数并清零；16 位强转自然处理回绕 */
static int16_t read_and_clear(TIM_HandleTypeDef *htim)
{
    int16_t cnt = (int16_t)__HAL_TIM_GET_COUNTER(htim);
    __HAL_TIM_SET_COUNTER(htim, 0u);
    return cnt;
}

void Encoder_Init(void)
{
    __HAL_TIM_SET_COUNTER(&htim3, 0u);
    __HAL_TIM_SET_COUNTER(&htim2, 0u);
    (void)HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    (void)HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
}

int16_t Encoder_ReadLeft(void)
{
    return (int16_t)(ENC_L_SIGN * read_and_clear(&htim3));
}

int16_t Encoder_ReadRight(void)
{
    return (int16_t)(ENC_R_SIGN * read_and_clear(&htim2));
}

void Encoder_Reset(void)
{
    (void)read_and_clear(&htim3);
    (void)read_and_clear(&htim2);
}
