/*
 * stm32f1xx_hal_msp.c —— MSP 初始化
 *
 * 本项目用 ADC1，所以除了全局 MSP，还要实现 HAL_ADC_MspInit：
 * 给 ADC1 开时钟，并把 PA0 配成「模拟输入」。
 * 注意：ADC 的时钟默认是 PCLK2/2 = 4MHz（复位默认值，在 0.6~14MHz 合法范围内），
 *       所以这里不用额外设置分频。
 */
#include "main.h"

void HAL_MspInit(void)
{
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
}

/* ADC 的底层初始化：HAL_ADC_Init() 会回头调用它 */
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (hadc->Instance == ADC1)
  {
    __HAL_RCC_ADC1_CLK_ENABLE();     /* 给 ADC1 开时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();    /* PA0 在 GPIOA 上，也要开 */

    /* 模拟输入引脚不需要上下拉、也不需要速度配置 */
    GPIO_InitStruct.Pin = GPIO_PIN_0;        /* PA0 -> ADC1_IN0 */
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG; /* 模拟模式 */
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
  if (hadc->Instance == ADC1)
  {
    __HAL_RCC_ADC1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);
  }
}
