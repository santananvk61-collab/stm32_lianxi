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
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp_at8236.h"
#include "bsp_encoder.h"
#include "bsp_adc_vbat.h"
#include "bsp_uart.h"
#include "bsp_spi_icm42670.h"
#include "attitude.h"
#include "balance.h"
#include "protocol.h"
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
/* 低压锁存：<9.0V 锁停，>9.9V 解锁 */
static uint8_t s_vbat_lock = 0u;
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
  MX_TIM4_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_SPI1_Init();
  MX_TIM5_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  /* ---- 控制时基 5ms：84MHz/84/5000，覆盖 CubeMX 的 1ms（不改 .ioc） ---- */
  htim5.Init.Prescaler = 83u;
  htim5.Init.Period    = 4999u;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK) {
    Error_Handler();
  }

  /* ---- 板级驱动 ---- */
  Motor_Init();        /* TIM4 四路 PWM */
  Motor_Brake();       /* 上电先刹车 */
  Encoder_Init();      /* TIM3/TIM2 正交编码器 */
  Vbat_Init();
  Uart_Init();         /* USART1 中断接收 */
  Protocol_Init();

  /* ---- IMU 自检：失败每 200ms 重试（堵转拉塌 3.3V 会导致传感器掉线） ---- */
  while (ICM_Init() != 0u) {
    Uart_SendString("ICM42670 retry...\r\n");
    HAL_Delay(200);
  }
  Uart_SendString("ICM42670 OK\r\n");

  /* ---- 上电稳定 1.5s，期间车必须静止 ---- */
  HAL_Delay(1500);

  /* ---- 陀螺零偏校准（阻塞约 2s），失败最多重试 3 次 ---- */
  Attitude_Init();
  {
    uint8_t cal_ok = 0u;
    uint8_t try;
    for (try = 0u; try < 3u; try++) {
      if (Attitude_Calibrate() == 0u) {
        cal_ok = 1u;
        break;
      }
      Uart_SendString("GYRO CAL FAIL, retry...\r\n");
      HAL_Delay(300);
    }
    if (cal_ok) {
      Uart_SendString("GYRO CAL OK\r\n");
    } else {
      /* 始终失败则 Balance_Init 停在 CAL 态（仅刹车、不读 SPI），不会误启动 */
      Uart_SendString("GYRO CAL FAIL (check IMU)\r\n");
    }
  }

  /* ---- 待机：刹车，立正静止 1s 自动启动 ---- */
  Balance_Init();
  HAL_TIM_Base_Start_IT(&htim5);
  Uart_SendString("HOLD UPRIGHT 1s TO START\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    Protocol_Task();   /* 指令解析 + 50ms 波形上传 */

    /* 电池监测：200ms 采样；<9.9V 提示，<9.0V 锁停，>9.9V 解锁 */
    {
      static uint32_t s_t_vbat = 0u;
      static uint32_t s_t_warn = 0u;
      uint32_t now = HAL_GetTick();

      if ((now - s_t_vbat) >= 200u) {
        float vb;
        s_t_vbat = now;
        vb = Vbat_Read();
        Balance_NotifyVbat(vb);

        if (vb < 9.0f) {
          if (!s_vbat_lock) {
            s_vbat_lock = 1u;
            Uart_SendString("VBAT STOP 9.0V\r\n");
          }
        } else if (vb > 9.9f) {
          s_vbat_lock = 0u;
        } else if ((vb < 9.9f) && ((now - s_t_warn) >= 5000u)) {
          s_t_warn = now;
          Uart_SendString("VBAT LOW 9.9V\r\n");
        }

        /* 锁定期间持续回待机，清掉启动计时，避免反复自动 arm */
        if (s_vbat_lock) {
          Balance_Stop();
        }
      }
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
/* TIM5 5ms 更新中断（优先级最高），直接执行控制链 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM5) {
    Balance_Step();
  }
}
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
