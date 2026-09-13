/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */
/* ================= 引脚接线说明 =================
 * L298N 电机驱动（四轮，左右各 2 个轮子并在一起）：
 *   左轮组 ENA(PWM) -> PA6 (TIM3_CH1)
 *   左轮组 IN1 -> PA4, IN2 -> PA5
 *   右轮组 ENB(PWM) -> PA7 (TIM3_CH2)
 *   右轮组 IN3 -> PB0, IN4 -> PB1
 * HC-SR04 超声波：
 *   TRIG -> PB5
 *   ECHO -> PB6
 * ================================================== */

/* ---------- 电机方向引脚（L298N 的 IN 口） ---------- */
#define MOTOR_L_IN1_PIN      GPIO_PIN_4
#define MOTOR_L_IN1_PORT     GPIOA
#define MOTOR_L_IN2_PIN      GPIO_PIN_5
#define MOTOR_L_IN2_PORT     GPIOA

#define MOTOR_R_IN3_PIN      GPIO_PIN_0
#define MOTOR_R_IN3_PORT     GPIOB
#define MOTOR_R_IN4_PIN      GPIO_PIN_1
#define MOTOR_R_IN4_PORT     GPIOB

/* ---------- 超声波引脚（HC-SR04） ---------- */
#define ULTRASONIC_TRIG_PIN     GPIO_PIN_5
#define ULTRASONIC_TRIG_PORT    GPIOB
#define ULTRASONIC_ECHO_PIN     GPIO_PIN_6
#define ULTRASONIC_ECHO_PORT    GPIOB

/* ---------- 避障参数 ---------- */
#define AVOID_DISTANCE_CM   20    /* 前方障碍小于这个距离就避让（厘米） */
#define CAR_SPEED           60    /* 直行速度（0~100） */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
