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
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
  #include "oled.h"
  #include "stdio.h"
  #include "string.h"


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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* ===== 以下为【原代码】，已注释保留，方便你对照 ===== */
// int up = 0;                     // 【原】普通全局变量：上升沿捕获值（没加 volatile，中断里改它可能不更新）
// int down = 0;                   // 【原】普通全局变量：下降沿捕获值
// float distance = 0;             // 【原】普通全局变量：算出来的距离
//   void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)   // 【原】输入捕获中断回调（旧版）
//   {
//     if((htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) && (htim == &htim1))//
//     {
//       up = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_3);    // 【原】读通道3捕获值
//       down = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_4);  // 【原】读通道4捕获值
//       distance = (down - up) * 0.034 / 2;                      // 【原】距离公式（你已改对符号）
//     }
//   }
/* ===== 原代码结束 ===== */

/* ===== 以下为【新代码】：按钮模拟回波，逐行注释 ===== */
volatile int up = 0;                  // 全局变量：松开时读到的计数值(=按住时长µs)；volatile 告诉编译器它会被中断改，别优化掉
volatile int down = 0;                // 全局变量：按下时读到的计数值（仅作记录/显示用）
volatile float distance = 0;          // 全局变量：由按住时长换算出的“模拟距离”
static uint32_t last_edge_ms = 0;     // 静态变量：上一次有效边沿的时间戳(ms)，用于软件消抖
static uint8_t pressed = 0;           // 静态变量：标志位，1=已经记录过“按下”、在等“松开”

// 输入捕获中断回调函数：定时器在 PA10 上抓到上升/下降沿时，硬件自动进这里
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim != &htim1) return;         // 只处理 TIM1 的捕获，别的定时器进来直接退出（防止误处理）

  uint32_t now = HAL_GetTick();       // 读取当前系统时间(ms)，HAL_GetTick() 是 HAL 提供的毫秒计数器
  if (now - last_edge_ms < 30) return;// 软件消抖：距上次有效边沿不足30ms的边沿(按钮抖动)直接忽略
  last_edge_ms = now;                 // 记录本次边沿时间，作为下一次消抖的基准

  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)            // 通道4=下降沿（上拉时：按下按钮=变低=下降沿）
  {
    down = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_4); // 读下此刻计数器值（先存着，紧接着就清零）
    __HAL_TIM_SET_COUNTER(&htim1, 0);                       // 把计数器清零：让“按住时长”从 0 开始算
    pressed = 1;                                            // 打标记：已按下，等松开时再算距离
  }
  else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)       // 通道3=上升沿（上拉时：松开按钮=变高=上升沿）
  {
    if (pressed)                                            // 只有确实经历过“按下”才处理，避免误触发
    {
      up = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_3);  // 读计数器=从按下到松开经过的 µs 数
      distance = up * 0.034f / 2.0f;                         // 换算成“距离”：声速0.034cm/µs，÷2是声波往返
      pressed = 0;                                           // 清标记，等待下一次按下
    }
  }
}
/* ===== 新代码结束 ===== */

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
  MX_I2C1_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(20);                     // 上电后延时20ms，等外设(oled/i2c)稳定
  SSD1306_Init();                    // 初始化OLED屏幕

  HAL_TIM_Base_Start(&htim1);        // 启动TIM1时基(计数器开始跑)，输入捕获依赖它
  
  char str[64] = "";                 // 显示用字符串缓冲区，64字节足够"juli:xx.xxcm up:x down:x"

  	  //根据手册启动模块，启动后会离开输出一个高电平，然后被定时器通道3捕获
  	  // HAL_Delay(1000);
      // HAL_GPIO_WritePin(Trip_GPIO_Port, Trip_Pin, GPIO_PIN_SET);
      // HAL_Delay(100);
      // HAL_GPIO_WritePin(Trip_GPIO_Port, Trip_Pin, GPIO_PIN_RESET);
      // HAL_Delay(1000);

      // if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_10) == GPIO_PIN_SET)
      //{
        // HAL_Delay(20);
        // if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_10) == GPIO_PIN_SET)
        //{
          // __HAL_TIM_SET_COUNTER(&htim1, 0);//启动模块后清零
        //}
     //}

      // if(__HAL_TIM_GET_COUNTER(&htim1) == 0)
      //{
    	  //启动定时器通道3，会在触发后记录数值到输入捕获寄存器，已经在mx中设置为上升沿触发
    	  // HAL_TIM_IC_Start(&htim1, TIM_CHANNEL_3);
    	  //启动定时器通道4，会在触发后记录数值到输入捕获寄存器，并在触发后触发中断，已经在mx中设置为下降沿触发
    	  // HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_4);
      //}

      
  /* —— 以下是【新代码】：直接启动两个通道的输入捕获中断（按下/松开都会进回调） —— */
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_3);   // 启动通道3输入捕获并开中断：抓“上升沿”(松开按钮)
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_4);   // 启动通道4输入捕获并开中断：抓“下降沿”(按下按钮)

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)                                   // 主循环：反复刷新屏幕，显示最新测量结果
  {

      HAL_Delay(20);                         // 每轮延时20ms，控制刷新率(约50Hz)，也让CPU歇口气

      SSD1306_Clear();                       // 清屏：把显存清成全黑，准备画新内容
      // sprintf(str, "juli:%.2fcm,up:%d,down:%d", distance,up,down);  // 【原】无长度保护，可能溢出
      snprintf(str, sizeof(str), "juli:%.2fcm,up:%d,down:%d", distance,up,down); // 【新】限定最多写sizeof(str)字节，绝不会溢出
      SSD1306_SetCursor(0, 0);               // 设置文字起始坐标(第0列,第0行)
      SSD1306_WriteString(str);              // 把字符串写进显存
      SSD1306_UpdateScreen();                // 把显存推到OLED屏上，真正显示出来
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
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
