#include "delay.h"

/* 主频 72MHz，1us 需要 72 个计数周期 */
#define TICKS_PER_US   (SystemCoreClock / 1000000U)

/*
 * DWT（数据观察点与跟踪）里有个 CYCCNT 计数器，每个 CPU 周期 +1。
 * 用它算延时比软件循环精确，之前搞 DHT11 单总线也是这套思路。
 */
void Delay_Init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;  /* 打开 DWT 跟踪 */
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;             /* 启动 CYCCNT 计数 */
}

void delay_us(uint32_t us)
{
  uint32_t start = DWT->CYCCNT;
  uint32_t ticks = us * TICKS_PER_US;
  while ((DWT->CYCCNT - start) < ticks)
  {
  }
}
