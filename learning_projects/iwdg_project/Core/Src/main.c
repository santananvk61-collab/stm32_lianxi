/*
 * iwdg_project —— 第 9 个外设：IWDG（独立看门狗）
 *
 * 学习目标：给程序挂一条「看门狗」——如果主循环卡死（死等外设、数组越界跑飞、
 * 死循环出不去了），看门狗计满没人喂，就把整块芯片复位拉起来，系统自己救自己。
 *
 * 看门狗的原理就一句话：
 *   一个独立递减计数器，从 Reload 值往下数，数到 0 就强制复位；
 *   程序必须在它数到 0 之前「喂狗」（重装计数），狗饿不死 = 程序还活着。
 *
 * 为什么叫「独立」：它跑在 LSI（内部低速 RC，约 40kHz）上，跟主时钟 HSI/PLL
 * 完全无关——主时钟就算彻底跑挂了，看门狗照样在数数，照样能复位芯片。
 *
 * 本程序做的事：
 *   1) 上电先快速闪 3 下 LED（「开机指纹」，用来肉眼判断板子有没有被复位）
 *   2) 启动 IWDG，超时时间 1 秒
 *   3) 主循环每 100ms 喂一次狗 + 翻转 LED —— 灯一直闪 = 程序活着
 *
 * 实验（看门狗真的会咬人吗）：
 *   把 while(1) 里的 HAL_IWDG_Refresh 那行注释掉再烧录 ——
 *   板子会每约 1 秒复位一次：你看到的不再是持续闪烁，
 *   而是「开机 3 连闪 → 亮灭一小会 → 又 3 连闪」不断重复。
 */
#include "main.h"

IWDG_HandleTypeDef hiwdg;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_IWDG_Init(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  /* 开机指纹：快速闪 3 下。被看门狗复位后也会走到这里，肉眼就能分辨 */
  for (int i = 0; i < 3; i++)
  {
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    HAL_Delay(80);
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    HAL_Delay(80);
  }

  /*
   * 注意顺序：看门狗放在最后再启动。
   * IWDG 一旦启动就无法用软件关闭（只能靠复位），先初始化别的再放狗。
   */
  MX_IWDG_Init();

  while (1)
  {
    HAL_IWDG_Refresh(&hiwdg);                  /* 喂狗：把计数器重装回 1250 */

    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    HAL_Delay(100);                            /* 循环周期 100ms << 1s 超时，狗饿不死 */
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

/* GPIO 初始化：板载 LED，PC13 推挽输出（低电平点亮） */
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

/*
 * IWDG 初始化：超时时间怎么算？
 *   LSI ≈ 40kHz（手册典型值，实际 30~60kHz 都有可能，所以别指望它精确定时）
 *   预分频 32  →  计数频率 = 40000 / 32 = 1250Hz（数一个数 0.8ms）
 *   重装值 1250 →  超时时间 ≈ (1250 + 1) / 1250Hz ≈ 1 秒
 *   通用公式：超时 ≈ (Reload + 1) × 预分频 ÷ 40000
 */
static void MX_IWDG_Init(void)
{
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;   /* 40000Hz / 32 = 1250Hz */
  hiwdg.Init.Reload = 1250;                   /* 1250 个计数 ≈ 1 秒 */
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
    Error_Handler();
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
