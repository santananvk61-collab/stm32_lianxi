/*
 * stm32f1xx_hal_msp.c —— MSP 初始化
 *
 * 本项目只用了 GPIO（DHT11 数据脚 PA0 + LED PC13），
 * GPIO 的时钟在 main.c 的 MX_GPIO_Init 里开，这里只有全局 MSP。
 */
#include "main.h"

void HAL_MspInit(void)
{
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
}
