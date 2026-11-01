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
#include "dma.h"
#include "i2c.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "aht20.h"
#include <stdio.h>
#include "bmp280.h"
#include "error_code.h"
#include "zj_rtc.h"

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
/* AHT20 输出 float,BMP280 补偿函数输出 double,类型要和驱动一致 */
static float  aht_temp = 0.0f;
static float  aht_hum  = 0.0f;
static double bmp_temp = 0.0;
static double bmp_pressure = 0.0;   /* 单位 Pa,显示时换算成 hPa */

/* 4行显示缓存:屏幕每行最多21个ASCII字符,24字节留余量(旧代码20字节会触发sprintf溢出警告) */
static char line1[30];
static char line2[24];
static char line3[24];
static char line4[24];

static uint8_t last_hour = 0xFF;   // 初值给0xFF,保证上电第一次!=当前小时
//static uint8_t first_run = 1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  ZJ_RTC_SetDateTime(2026, 9, 13, 14, 4, 0);

  
  while (1)
  {
    DeviceError_t err;
    err = AHT20_Init();
    if(err != ERR_OK)
    {
      if(err == ERR_NOT_RESPOND) I2C1_Recover();
      OLED_Clear();
      OLED_SetCursor(2, 18);
      sprintf(line2, "AHT20 %s", ErrorToStr(err));
      OLED_ShowString(line2);
      OLED_Update();
      HAL_Delay(500);
      continue;
    }

    err = BMP280_Init();
    if(err != ERR_OK)
    {
      if(err == ERR_NOT_RESPOND) I2C1_Recover();
      OLED_Clear();
      OLED_SetCursor(2, 32);
      sprintf(line3, "BMP280 %s", ErrorToStr(err));
      OLED_ShowString(line3);
      OLED_Update();
      HAL_Delay(500);
      continue;
    }

    break;   /* 两个设备都就绪 */
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    RTC_DateTypeDef d;
    RTC_TimeTypeDef t;
    if (ZJ_RTC_GetDateTime(&d, &t) == ERR_OK)
    {
      sprintf(line1, "20%02d-%02d-%02d %02d:%02d:%02d", d.Year, d.Month, d.Date, t.Hours, t.Minutes, t.Seconds);
    }
    else
    {
      sprintf (line1, "RTC Error");
    }

    DeviceError_t aht_err = AHT20_Read(&aht_temp, &aht_hum);
    if (aht_err == ERR_OK)
    {
      sprintf (line2, "温度: %.2f ℃", aht_temp);
      sprintf (line3, "湿度: %.2f %%", aht_hum);
    }
    else
    {
      sprintf (line2, "AHT20 Error");
      sprintf (line3, "AHT20 Error");
    }

    DeviceError_t bmp_err = BMP280_Read(&bmp_pressure, &bmp_temp);
    if (bmp_err == ERR_OK)
    {
      sprintf (line4, "气压: %.2f hPa", bmp_pressure/100.0);
    }
    else
    {
      sprintf (line4, "BMP280 Error");
    }

    OLED_Clear();
    OLED_SetCursor(2,2);
    OLED_ShowString(line1);
    OLED_SetCursor(2, 18);
    OLED_ShowString(line2);
    OLED_SetCursor(2, 32);
    OLED_ShowString(line3);
    OLED_SetCursor(2, 46);
    OLED_ShowString(line4);
    OLED_Update();

    /* I2C 通信失败时复位外设,清掉主机侧残留状态 */
    if (aht_err == ERR_NOT_RESPOND || bmp_err == ERR_NOT_RESPOND)
    {
      I2C1_Recover();
    }

    if (t.Hours != last_hour)
    {
      /* 串口调试:成功时打印时间戳+原始码值+换算值,供校准比对 */
      if (aht_err == ERR_OK && bmp_err == ERR_OK)
      {
      uint32_t aht_rt, aht_rh;
      int32_t  bmp_rp, bmp_rt;
      AHT20_GetRaw(&aht_rt, &aht_rh);
      BMP280_GetRaw(&bmp_rp, &bmp_rt);
      Debug_Print("[%s] AHT raw T=%lu H=%lu | %.2fC %.2f%% | BMP raw P=%ld T=%ld | %.2fhPa %.2fC\r\n",
                  line1, aht_rt, aht_rh, aht_temp, aht_hum,
                  bmp_rp, bmp_rt, bmp_pressure/100.0, bmp_temp);
      }
    }

    last_hour = t.Hours;
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
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
