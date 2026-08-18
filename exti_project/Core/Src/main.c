/*
 * exti_project —— 第 5 个外设：EXTI（外部中断）
 *
 * 学习目标：让引脚电平的变化「打断」CPU，立刻去处理——而不是靠死循环一直问。
 * 这跟轮询（polling）相反：轮询是 CPU 反复问「变了吗？」，中断是引脚主动说「我变了！」。
 *
 * 本程序做的事：
 *   按键接在 PA1，按下（下降沿）触发 EXTI1 中断，
 *   在中断回调里翻转板载 LED。主循环里什么都不用做。
 *
 * 中断是怎么跑起来的（三步）：
 *   1) 把 PA1 配成「中断输入」（GPIO_MODE_IT_FALLING）
 *   2) 配置中断优先级并「开门」使能它（HAL_NVIC_SetPriority / EnableIRQ）
 *   3) 写好中断处理函数（it.c 里的 EXTI1_IRQHandler -> HAL_GPIO_EXTI_IRQHandler）
 *      它最后会调用下面的 HAL_GPIO_EXTI_Callback，我们在回调里做真正的事。
 */
#include "main.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  /* 主循环空着：按键的事交给中断去处理 */
  while (1)
  {
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

/* GPIO 初始化：LED 输出 + 按键中断输入 + 使能 EXTI1 中断 */
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

  /* 按键：PA1 下降沿中断（上拉，按下时从高变低 = 下降沿） */
  GPIO_InitStruct.Pin = KEY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(KEY_GPIO_Port, &GPIO_InitStruct);

  /* 配置并「打开」EXTI1 的中断（不使能的话，引脚变化不会打断 CPU） */
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
}

/* 外部中断回调：按下按键后，会走到这里 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == KEY_Pin)
  {
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);  /* 按一下，灯翻转一次 */
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
