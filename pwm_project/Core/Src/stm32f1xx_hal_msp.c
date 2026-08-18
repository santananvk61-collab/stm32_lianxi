/*
 * stm32f1xx_hal_msp.c —— MSP 初始化
 *
 * 本项目用 TIM3 输出 PWM，HAL_TIM_PWM_MspInit 负责：
 *   给 TIM3 开时钟，并把 PA6（TIM3_CH1）配成复用推挽输出。
 */
#include "main.h"

void HAL_MspInit(void)
{
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
}

/* PWM 定时器的底层初始化：HAL_TIM_PWM_Init() 会回头调用它 */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_pwm)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (htim_pwm->Instance == TIM3)
  {
    __HAL_RCC_TIM3_CLK_ENABLE();    /* 给 TIM3 开时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();   /* PA6 在 GPIOA 上 */

    /* PA6 -> TIM3_CH1：复用推挽输出（输出 PWM 波形） */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* htim_pwm)
{
  if (htim_pwm->Instance == TIM3)
  {
    __HAL_RCC_TIM3_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_6);
  }
}
