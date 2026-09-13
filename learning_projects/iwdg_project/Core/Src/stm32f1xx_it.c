/*
 * stm32f1xx_it.c —— 中断服务函数（ISR）集合
 *
 * 当某个中断发生，CPU 会根据「向量表」跳到这里的对应函数。
 * 本工程只用 SysTick，没有额外的外设中断（IWDG 不产生中断，超时直接复位）。
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

/* SysTick 滴答定时器中断：HAL 库靠它计数，提供 HAL_Delay 的毫秒延时 */
void SysTick_Handler(void)
{
  HAL_IncTick();
}
