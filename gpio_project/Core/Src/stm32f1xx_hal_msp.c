/*
 * stm32f1xx_hal_msp.c —— MSP（MCU Support Package）初始化
 *
 * MSP 是「底层支持包」：当 HAL 库初始化某个外设时，会回头调用这里对应的
 * HAL_xxx_MspInit 函数，由我们把「时钟、引脚、中断」这些硬件细节配好。
 * 本项目只用 GPIO（时钟在 main.c 的 MX_GPIO_Init 里开了），所以这里只有全局 MSP。
 */
#include "main.h"

/* 全局 MSP：上电后由 HAL_Init() 调用一次 */
void HAL_MspInit(void)
{
  __HAL_RCC_AFIO_CLK_ENABLE();   /* AFIO：外部中断/引脚重映射要用到 */
  __HAL_RCC_PWR_CLK_ENABLE();    /* PWR：电源控制 */
}
