/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart3_tx;

/* USER CODE BEGIN PV */
/* ---- 健壮接收：缓冲累加 + 帧头定位 + 超时兜底 ---- */
#define RX_BUF_SIZE      128        // 重组缓冲，比单帧大一些
#define FRAME_TIMEOUT_MS 100         // 超过这个时间还没凑齐一帧，就丢弃残帧重同步

uint8_t rx_buf[RX_BUF_SIZE];         // 跨多次 IDLE 累积的重组缓冲
uint8_t rx_len = 0;                  // 缓冲里已收到的字节数
uint32_t last_rx_tick = 0;           // 上次收到字节的时间戳（HAL_GetTick）

uint8_t received_data[64];//数据缓存区
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* 从重组缓冲里抽出“已经凑齐”的完整帧并执行；抽完一帧就把它从缓冲移除 */
static void ParseFrames(void)
{
    while (rx_len > 0)
    {
        /* 1) 不是帧头 0xAA → 丢掉这一字节，后面的往前移（自动跳过垃圾/干扰） */
        if (rx_buf[0] != 0xAA)
        {
            memmove(rx_buf, rx_buf + 1, rx_len - 1);
            rx_len--;
            continue;
        }

        /* 2) 至少要有“长度字节”才能知道整帧多大 */
        if (rx_len < 2)
            return;

        uint8_t frame_len = rx_buf[1];   // 第2字节 = 整帧长度

        /* 长度明显非法（过小或过大）→ 当无效帧头，丢弃 0xAA 重同步 */
        if (frame_len < 2 || frame_len > RX_BUF_SIZE)
        {
            memmove(rx_buf, rx_buf + 1, rx_len - 1);
            rx_len--;
            continue;
        }

        /* 3) 缓冲里还没凑齐这一整帧 → 先返回，等下次 IDLE 再来 */
        if (rx_len < frame_len)
            return;

        /* 4) 校验和：前面所有字节相加 == 最后一字节 */
        uint8_t sum = 0;
        for (int i = 0; i < frame_len - 1; i++)
            sum += rx_buf[i];

        if (sum == rx_buf[frame_len - 1])   // 校验通过才执行
        {
            for (int i = 2; i < frame_len - 1; i += 2)
            {
                GPIO_PinState state = GPIO_PIN_RESET;
                if (rx_buf[i + 1] == 0xFF)       state = GPIO_PIN_SET;
                else if (rx_buf[i + 1] == 0x00)  state = GPIO_PIN_RESET;

                if (rx_buf[i] == 0x01)       HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1,  state);
                else if (rx_buf[i] == 0x02)  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, state);
                else if (rx_buf[i] == 0x03)  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4,  state);
            }
        }

        /* 不管校验过没过，这一整帧都从缓冲移除，接着解析下一帧 */
        memmove(rx_buf, rx_buf + frame_len, rx_len - frame_len);
        rx_len -= frame_len;
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart == &huart3)
    {
        /* 把这次 IDLE 收到的“一段”追加到重组缓冲（防溢出） */
        if (rx_len + Size <= RX_BUF_SIZE)
        {
            memcpy(rx_buf + rx_len, received_data, Size);
            rx_len += Size;
        }
        else
        {
            rx_len = 0;   // 缓冲满了，丢弃重来
        }

        last_rx_tick = HAL_GetTick();   // 记录“刚收到字节”的时间
        ParseFrames();                  // 试着从缓冲里抽出完整帧

        HAL_UART_Transmit_DMA(&huart3, received_data, Size);   // 原样回显（不变）

        HAL_UARTEx_ReceiveToIdle_DMA(&huart3, received_data, sizeof(received_data));
        __HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, received_data, sizeof(received_data));//接收不定长数据
    //DMA模式多一个传输过半中断
    //DMA关闭函数 选择DMA地址（哪个串口，rx还是tx），选择函数这里为传输过半中断函数
    __HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* 超时兜底：收了一半却迟迟凑不齐（残帧/干扰），丢弃重同步 */
    if (rx_len > 0 && (HAL_GetTick() - last_rx_tick) > FRAME_TIMEOUT_MS)
    {
        rx_len = 0;
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_RED_Pin|LED_YELLOW_Pin|LED_GREEN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_RED_Pin LED_YELLOW_Pin LED_GREEN_Pin */
  GPIO_InitStruct.Pin = LED_RED_Pin|LED_YELLOW_Pin|LED_GREEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
