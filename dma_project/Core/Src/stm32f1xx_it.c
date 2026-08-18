/*
 * stm32f1xx_it.c —— 中断服务函数（ISR）集合
 *
 * 本项目的重点是 DMA1 通道 7 的中断：DMA 搬运完成后触发它，
 * 交给 HAL_DMA_IRQHandler 去收尾（清标志、通知串口「发完了」）。
 */
#include "main.h"
#include "stm32f1xx_it.h"

void NMI_Handler(void)
{
  while (1)
  {
  }
}

void HardFault_Handler(void)
{
  while (1)
  {
  }
}

void MemManage_Handler(void)
{
  while (1)
  {
  }
}

void BusFault_Handler(void)
{
  while (1)
  {
  }
}

void UsageFault_Handler(void)
{
  while (1)
  {
  }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
  HAL_IncTick();
}

/* DMA1 通道 7 中断：USART2_TX 搬完了 */
void DMA1_Channel7_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart2_tx);
}
