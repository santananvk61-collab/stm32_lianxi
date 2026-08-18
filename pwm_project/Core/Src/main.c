/*
 * pwm_project —— 第 4 个外设：PWM（脉宽调制）
 *
 * 学习目标：用「开关开关开关……」这种方波的平均效果，来调灯的亮度或电机的转速。
 * 比如同样是 1kHz 的方波，高电平占 90% 灯就亮，占 10% 灯就暗——这就是「占空比」。
 *
 * 本程序做的事：让接在 PA6 上的 LED 做「呼吸灯」——由暗到亮、再由亮到暗循环。
 *
 * 两个关键参数：
 *   频率 = 定时器时钟 / (分频+1) / (周期+1)
 *        = 8MHz / 8 / 1000 = 1kHz   （分频 7，周期 999）
 *   占空比 = 比较值 / 周期 = Pulse / 1000   （改 Pulse 就能调亮度）
 */
#include "main.h"

TIM_HandleTypeDef htim3;

void SystemClock_Config(void);
static void MX_TIM3_Init(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_TIM3_Init();

  int pulse = 0;   /* 当前比较值 = 亮度（0 最暗，999 最亮） */
  int dir = 1;     /* 方向：+1 变亮，-1 变暗 */

  while (1)
  {
    /* 每次加/减 5，到达两端就掉头 */
    pulse += dir * 5;
    if (pulse >= 999) { pulse = 999; dir = -1; }
    if (pulse <= 0)   { pulse = 0;   dir = 1; }

    /* 把新的比较值写进去 = 更新占空比 = 更新亮度 */
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);

    HAL_Delay(5);   /* 5ms 一步，呼吸一个周期约 2 秒 */
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

/* TIM3 初始化：PWM 模式，通道 1 输出到 PA6 */
static void MX_TIM3_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 7;                        /* 8MHz / 8 = 1MHz */
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;     /* 向上计数 */
  htim3.Init.Period = 999;                         /* 1MHz / 1000 = 1kHz */
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
    Error_Handler();

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
    Error_Handler();

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
    Error_Handler();

  /* 通道 1 的 PWM 参数 */
  sConfigOC.OCMode = TIM_OCMODE_PWM1;          /* PWM 模式 1 */
  sConfigOC.Pulse = 0;                          /* 初始占空比 0%（最暗） */
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;   /* 高电平有效 */
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    Error_Handler();

  /* 启动 PWM 输出（不启动的话，配置了也不会出波形） */
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
