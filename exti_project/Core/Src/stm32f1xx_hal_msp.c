/*
 * stm32f1xx_hal_msp.c —— MSP 初始化
 *
 * 外部中断用到了 AFIO（EXTI 的引脚映射寄存器在 AFIO 里），
 * 所以这里一定要开 AFIO 时钟，否则 PA1 的中断线映射不生效。
 */
#include "main.h"

void HAL_MspInit(void)
{
  __HAL_RCC_AFIO_CLK_ENABLE();   /* 外部中断的引脚映射需要 AFIO */
  __HAL_RCC_PWR_CLK_ENABLE();
}
