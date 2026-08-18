/*
 * adc_project —— 第 2 个外设：ADC（模数转换器）
 *
 * 学习目标：把「模拟电压」变成「数字量」。
 * 现实世界的光、温度、旋钮电压都是连续的模拟量，MCU 只能懂数字，
 * 所以需要 ADC 来「量」一下这根引脚上的电压是多少伏。
 *
 * 本程序做的事：
 *   用 ADC1 的通道 0（PA0）读一个模拟电压，换算成毫伏(mV)，
 *   再用这个电压控制板载 LED 闪烁的快慢——电压越高闪得越快。
 *   （这样不接串口也能用肉眼「看」到 ADC 读数。）
 *
 * 关键换算公式（背下来）：
 *   12 位 ADC 的读数范围是 0 ~ 4095，对应 0V ~ 3.3V（基准电压）。
 *   电压(mV) = 读数 * 3300 / 4095
 */
#include "main.h"

ADC_HandleTypeDef hadc1;   /* ADC1 的句柄（HAL 库用来记住配置的结构体） */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_ADC1_Init();

  while (1)
  {
    uint16_t adc_raw = 0;   /* ADC 原始读数（0~4095） */
    uint32_t mv = 0;        /* 换算后的电压，单位毫伏 */

    /* 1) 启动一次转换（软件触发） */
    HAL_ADC_Start(&hadc1);

    /* 2) 等它转换完（最多等 100ms），然后读结果 */
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
    {
      adc_raw = HAL_ADC_GetValue(&hadc1);
      mv = (uint32_t)adc_raw * 3300 / 4095;   /* 核心换算：12位->电压 */
    }

    /* 3) 用完停掉（省电，也便于下次干净地重新启动） */
    HAL_ADC_Stop(&hadc1);

    /* 4) 用电压控制闪烁快慢：0V 时约 1 秒闪一次，3.3V 时约 50ms 闪一次 */
    uint32_t delay = 1000 - (mv * 950 / 3300);
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    HAL_Delay(delay);
  }
}

/* 系统时钟：内部 HSI 8MHz */
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

/* GPIO：板载 LED PC13 输出 */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);
}

/* ADC1 初始化：通道 0（PA0），单次软件触发 */
static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;         /* 不扫描，只采一个通道 */
  hadc1.Init.ContinuousConvMode = DISABLE;            /* 不连续转换：手动触发一次采一次 */
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;   /* 软件触发（调用 Start 就开转） */
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;         /* 结果右对齐 */
  hadc1.Init.NbrOfConversion = 1;                     /* 只转换 1 个通道 */
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
    Error_Handler();

  sConfig.Channel = ADC_CHANNEL_0;                    /* PA0 = ADC1 的通道 0 */
  sConfig.Rank = ADC_REGULAR_RANK_1;                  /* 第 1 个（也是唯一一个） */
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;  /* 采样时间越长，读数越稳 */
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    Error_Handler();
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
