/*
 * gpio_project —— 第 1 个外设：GPIO（通用输入输出）
 *
 * 学习目标：搞懂「怎么让一个引脚输出高低电平」。
 * 这是所有外设的地基——后面点灯、驱动电机、读按键，本质都是 GPIO。
 *
 * 本程序做的事：让板载 LED（PC13）以 500ms 周期闪烁。
 *
 * 套路（背下来，以后所有外设都是这 4 步）：
 *   1) 开时钟   —— 每个外设都要先给它供电（开 RCC 时钟）
 *   2) 配置     —— 告诉这个引脚「你是输出、推挽、速度多少」
 *   3) 用       —— 在主循环里写高/写低
 *   4) 死循环   —— while(1) 里不停重复（相当于 PLC 的循环扫描）
 */
#include "main.h"

/* 函数声明（先告诉编译器有这么几个函数，后面再实现） */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

int main(void)
{
  /* 1. 复位所有外设、初始化 Flash 和 SysTick 滴答定时器 */
  HAL_Init();

  /* 2. 配置系统时钟（这里用内部 HSI 8MHz，最简单、不依赖外部晶振） */
  SystemClock_Config();

  /* 3. 初始化我们用到的外设（这里只有 GPIO） */
  MX_GPIO_Init();

  /* 4. 主循环：翻转 LED，延时 500ms —— 灯就一闪一闪了 */
  while (1)
  {
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);  /* 翻转：亮<->灭 */
    HAL_Delay(500);                               /* 停 500 毫秒 */
  }
}

/* 系统时钟：使用内部 HSI 8MHz，APB1/APB2 都不分频 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;  /* 用内部高速振荡器 */
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;              /* 不用 PLL 倍频 */
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

/* GPIO 初始化：把 PC13 配成推挽输出 */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* 第 1 步：给 GPIOC 这个「端口」开时钟（不开时钟，寄存器写了也白写） */
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /* 初始状态先让它灭（低电平点亮，所以先写 1 = 灭） */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

  /* 第 2 步：填一个「配置表」结构体，告诉芯片 PC13 怎么工作 */
  GPIO_InitStruct.Pin   = LED_Pin;              /* 哪根引脚 */
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;  /* 推挽输出（能推电流也能吸电流） */
  GPIO_InitStruct.Pull  = GPIO_NOPULL;          /* 输出模式不需要上拉/下拉 */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;  /* 翻转速度：低（点灯足够） */
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);
}

/* 出错处理：关中断后死循环（调试时在这里打断点） */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
