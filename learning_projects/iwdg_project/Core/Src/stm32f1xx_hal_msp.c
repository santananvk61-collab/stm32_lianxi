/*
 * stm32f1xx_hal_msp.c —— MSP 初始化
 *
 * IWDG 走的是芯片内部的 LSI 时钟，不占用任何引脚，也不需要额外开时钟；
 * 这里照惯例开 AFIO / PWR 时钟保留（对 IWDG 无影响）。
 */
#include "main.h"

void HAL_MspInit(void)
{
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
}
