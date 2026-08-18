/*
 * dma_project —— 第 6 个外设：DMA（直接存储器访问）
 *
 * 学习目标：让数据「自己」流动，CPU 不用一个字节一个字节地搬。
 * 没有 DMA：CPU 每发一个字节都要亲自盯着串口寄存器；
 * 有 DMA：CPU 只说一句「把这块内存发出去」，DMA 就在后台自动搬完。
 *
 * 本程序做的事：
 *   每隔 1 秒，用 DMA 把一句话从内存搬到 USART2 发出去。
 *   打开串口助手（115200）就能看到这句话不断刷屏。
 *
 * 三个关键点：
 *   1) 方向 Direction = 内存 -> 外设（MEMORY_TO_PERIPH）
 *   2) 内存地址要递增（一个字节接一个字节），外设地址不动（就那一个数据寄存器）
 *   3) USART2_TX 固定用 DMA1 通道 7（芯片手册规定的映射）
 */
#include "main.h"
#include <string.h>

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_tx;

void SystemClock_Config(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_DMA_Init();
  MX_USART2_UART_Init();

  const char *msg = "STM32 DMA 搬运：这句话是 DMA 发出来的，CPU 没逐个字节参与\r\n";

  while (1)
  {
    /* CPU 只下一条指令，DMA 就在后台自动把整句话搬完 */
    HAL_UART_Transmit_DMA(&huart2, (uint8_t*)msg, strlen(msg));
    HAL_Delay(1000);   /* 等 1 秒，期间 DMA 在默默干活 */
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

/* DMA 初始化：配置 USART2 的发送通道 */
static void MX_DMA_Init(void)
{
  __HAL_RCC_DMA1_CLK_ENABLE();   /* 给 DMA1 开时钟 */

  /* USART2_TX 固定用 DMA1 通道 7（查芯片手册的映射表） */
  hdma_usart2_tx.Instance = DMA1_Channel7;
  hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;   /* 内存 -> 外设 */
  hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;       /* 外设地址不递增 */
  hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;           /* 内存地址递增 */
  hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;  /* 外设 1 字节 */
  hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;      /* 内存 1 字节 */
  hdma_usart2_tx.Init.Mode = DMA_NORMAL;                  /* 传完就停 */
  hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
    Error_Handler();

  /* 把这个 DMA 通道「挂」到串口的发送上，HAL_UART_Transmit_DMA 才能用 */
  __HAL_LINKDMA(&huart2, hdmatx, hdma_usart2_tx);

  /* 使能 DMA1 通道 7 的中断（传完后通知 HAL 库） */
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
}

/* USART2 初始化：115200 8N1 */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
    Error_Handler();
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
