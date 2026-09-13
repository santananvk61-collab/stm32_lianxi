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
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include <stdio.h>
#include <string.h>
#include "NTC.h"
#include <math.h>
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
  uint16_t adc_val[4];
  float Voltage = 0.0;
  float Vref = 0.0;
  char buf[64] = "";
  char buf1[64] = "";
  char buf2[64] = "";
  char buf3[64] = "";
  char buf4[64] = "";
//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)//转换完成回调：中断模式和DMA模式共用；DMA模式下，整轮4通道转换并全部搬运完毕后才进来一次
//{
//  if(hadc == &hadc1)
//  {
//    sprintf(buf, "ADC1: %d, %d, %d, %d\r\n", adc_val[0], adc_val[1], adc_val[2], adc_val[3]);
//    SSD1306_SetCursor(4, 4);
//    SSD1306_WriteString(buf);
//
//  }
//}
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
  MX_ADC1_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(20);
  SSD1306_Init();
  HAL_Delay(20);

  HAL_ADCEx_Calibration_Start(&hadc1);//校准ADC
  //开启循环模式后会自动轮询，不用回调函数了，且只需启动一次即可
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_val, sizeof(adc_val) / sizeof(adc_val[0]));//开启ADC转换（DMA模式）

  float NTC_R = 0.0;
  float NTC_T = 0.0;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    SSD1306_Clear();

    SSD1306_DrawLine(0,0,SSD1306_WIDTH-1,0,1);
    SSD1306_DrawLine(0,0,0,SSD1306_HEIGHT-1,1);
    SSD1306_DrawLine(SSD1306_WIDTH-1,0,SSD1306_WIDTH-1,SSD1306_HEIGHT-1,1);
    SSD1306_DrawLine(0,SSD1306_HEIGHT-1,SSD1306_WIDTH-1,SSD1306_HEIGHT-1,1);

    //HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_val, sizeof(adc_val) / sizeof(adc_val[0]));//开启ADC转换（DMA模式）

    NTC_R = ADC2Resistance(adc_val[1]);
    NTC_T = resistance2Temperature(NTC_R);

    Vref = 1.2 * (4095.0 / (float)adc_val[3]);
    Voltage = Vref * ((float)adc_val[0] / 4095.0);

    sprintf(buf, "NTC_R: %.1f\r\n",NTC_R);
    sprintf(buf1, "NTC_T: %.2f\r\n",NTC_T);
    sprintf(buf2, "Vref: %.1f\r\n",Vref);
    sprintf(buf3, "Voltage: %.2f\r\n",Voltage);
    sprintf(buf4, "ADC_V: %d\r\n",adc_val[3]);

    SSD1306_SetCursor(4, 4);
    SSD1306_WriteString(buf);

    SSD1306_SetCursor(4, 18);
    SSD1306_WriteString(buf1);

    SSD1306_SetCursor(4, 27);
    SSD1306_WriteString(buf4);

    SSD1306_SetCursor(4, 36);
    SSD1306_WriteString(buf2);

    SSD1306_SetCursor(4, 45);
    SSD1306_WriteString(buf3);

    SSD1306_UpdateScreen();

    HAL_Delay(100);

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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
