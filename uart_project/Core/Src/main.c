/*
 * uart_project —— 第 3 个外设：UART（串口通信）
 *
 * 学习目标：让 MCU 和电脑互相「说话」。
 * 串口是最常用的调试手段——程序里加一句 printf 就能在电脑上看到运行情况。
 * USART2 通过板上的 USB 转串口芯片连到电脑（PA2=TX 发送，PA3=RX 接收）。
 *
 * 本程序做的事：一个「回声」程序——
 *   电脑发一个字符下来，MCU 收到后原样发回去。
 *   打开串口助手，波特率 115200，敲键盘就能看到回声。
 *
 * 三个关键参数（收发双方必须一致，否则乱码）：
 *   波特率 115200、8 个数据位、无校验、1 个停止位。
 */
#include "main.h"
#include <string.h>

UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_USART2_UART_Init(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_USART2_UART_Init();

  uint8_t ch;   /* 接收到的 1 个字节 */

  /* 上电发一句欢迎语，用来确认串口已打通 */
  const char *msg = "UART 回声测试：随便敲一个字符，我会原样回给你\r\n";
  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);

  while (1)
  {
    /* 阻塞接收 1 个字节：一直等，直到收到数据 */
    HAL_UART_Receive(&huart2, &ch, 1, HAL_MAX_DELAY);
    /* 原样发回去 = 回声 */
    HAL_UART_Transmit(&huart2, &ch, 1, 100);
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

/* USART2 初始化：115200 8N1 */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;             /* 波特率 */
  huart2.Init.WordLength = UART_WORDLENGTH_8B;  /* 8 个数据位 */
  huart2.Init.StopBits = UART_STOPBITS_1;       /* 1 个停止位 */
  huart2.Init.Parity = UART_PARITY_NONE;        /* 无校验 */
  huart2.Init.Mode = UART_MODE_TX_RX;           /* 既收又发 */
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;  /* 不用硬件流控 */
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
