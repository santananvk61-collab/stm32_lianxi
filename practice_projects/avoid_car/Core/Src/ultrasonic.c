#include "ultrasonic.h"
#include "delay.h"

#define TICKS_PER_US   (SystemCoreClock / 1000000U)

void Ultrasonic_Init(void)
{
  /* TRIG/ECHO 引脚已经在 MX_GPIO_Init 里配好了，这里只启动 DWT 延时 */
  Delay_Init();
}

/*
 * 测一次距离：
 * 1. TRIG 拉高 20us 以上触发一次测量
 * 2. ECHO 会拉高，持续时间和距离成正比
 * 3. 用 DWT 数出 ECHO 高电平的时间，换算成距离
 *
 * 声速约 340m/s = 0.034 cm/us，声波走的是来回，所以距离要除以 2：
 *   距离(cm) = 时间(us) * 0.034 / 2 = 时间(us) * 0.017
 */
float Ultrasonic_GetDistance(void)
{
  uint32_t us;
  uint32_t timeout;

  /* 触发一次测量 */
  HAL_GPIO_WritePin(ULTRASONIC_TRIG_PORT, ULTRASONIC_TRIG_PIN, GPIO_PIN_SET);
  delay_us(20);
  HAL_GPIO_WritePin(ULTRASONIC_TRIG_PORT, ULTRASONIC_TRIG_PIN, GPIO_PIN_RESET);

  /* 等 ECHO 变高，加超时保护，防止一直卡在这 */
  timeout = 0;
  while (HAL_GPIO_ReadPin(ULTRASONIC_ECHO_PORT, ULTRASONIC_ECHO_PIN) == GPIO_PIN_RESET)
  {
    if (++timeout > 100000)
    {
      return 0.0f;   /* 超时，测不到 */
    }
  }

  /* 数 ECHO 高电平的时间 */
  uint32_t start = DWT->CYCCNT;
  while (HAL_GPIO_ReadPin(ULTRASONIC_ECHO_PORT, ULTRASONIC_ECHO_PIN) == GPIO_PIN_SET)
  {
  }
  uint32_t end = DWT->CYCCNT;

  us = (end - start) / TICKS_PER_US;   /* 微秒 */

  return (float)us * 0.017f;           /* 厘米 */
}
