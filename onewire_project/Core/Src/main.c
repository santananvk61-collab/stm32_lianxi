/*
 * onewire_project —— 第 8 个外设：单总线（OneWire，以 DHT11 为例）
 *
 * 学习目标：用「一根数据线」和一个外设通信（相比 I2C 的两根还省一根）。
 * 单总线的难点在「时序」：0 和 1 是靠高电平持续多久来区分的，
 * 所以需要精确的微秒级延时。
 *
 * 本程序做的事：
 *   读 DHT11 温湿度传感器（数据脚 PA0）。读到就慢闪 LED 表示成功，
 *   读不到（超时/校验错）就快闪 LED 表示失败。
 *
 * 关键点：
 *   1) 单总线双方都只能「拉低」，释放后靠上拉电阻回高，所以不会短路冲突。
 *   2) DHT11 的微秒延时用内核 DWT 计数器实现，比 HAL_Delay(1ms) 精确。
 */
#include "main.h"
#include "dht11.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  DHT11_Init();   /* 开启 DWT 微秒计时器（读 DHT11 需要） */

  int8_t temperature = 0;   /* 温度（℃） */
  uint8_t humidity = 0;     /* 湿度（%） */

  while (1)
  {
    DHT11_Status st = DHT11_Read(&temperature, &humidity);

    if (st == DHT11_OK)
    {
      /* 成功：慢闪（约 1 秒一周期） */
      HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
      HAL_Delay(500);
    }
    else
    {
      /* 失败：快闪（约 200ms 一周期），提示读取出错 */
      HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
      HAL_Delay(100);
    }
  }
}

/* 系统时钟：内部 HSI 8MHz（DHT11 驱动会自动按实际频率换算微秒延时） */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    Error_Handler();

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    Error_Handler();
}

/* GPIO：LED 输出 + DHT11 数据脚 PA0（开漏输出 + 上拉） */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* LED：PC13 推挽输出 */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /* DHT11 数据脚 PA0：开漏输出 + 上拉。
   * 写 0 = 拉低总线；写 1 = 松手（靠上拉电阻回高）；
   * 读  = 读总线真实电平。这是单总线的标准接法。 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
