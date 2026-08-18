/*
 * stm32f1xx_it.c —— 中断服务函数（ISR）集合
 *
 * 当某个中断发生，CPU 会根据「向量表」跳到这里的对应函数。
 * 我们只关心 EXTI1_IRQHandler，其余是系统异常处理（照着保留即可）。
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

/* PA1 外部中断服务函数：交给 HAL 去清标志并回调 HAL_GPIO_EXTI_Callback */
void EXTI1_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}
