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
/* ================= 引脚接线说明（照着这个接就行） =================
 * L298N 电机驱动：
 *   左轮 ENA(PWM) -> PA6 (TIM3_CH1)
 *   左轮 IN1 -> PA4, IN2 -> PA5
 *   右轮 ENB(PWM) -> PA7 (TIM3_CH2)
 *   右轮 IN3 -> PB0, IN4 -> PB1
 * TCRT5000 循迹（4路，从车头左边往右边排）：
 *   左外 -> PA0, 左内 -> PA1, 右内 -> PA2, 右外 -> PA3
 * HC-05 蓝牙（串口）：
 *   TX -> PA10(单片机RX), RX -> PA9(单片机TX)
 * ================================================================= */

/* ---------- 电机方向引脚（L298N 的 IN 口） ---------- */
#define MOTOR_L_IN1_PIN      GPIO_PIN_4
#define MOTOR_L_IN1_PORT     GPIOA
#define MOTOR_L_IN2_PIN      GPIO_PIN_5
#define MOTOR_L_IN2_PORT     GPIOA

#define MOTOR_R_IN3_PIN      GPIO_PIN_0
#define MOTOR_R_IN3_PORT     GPIOB
#define MOTOR_R_IN4_PIN      GPIO_PIN_1
#define MOTOR_R_IN4_PORT     GPIOB

/* ---------- 循迹传感器引脚（TCRT5000） ---------- */
/* 4 路：左外 L2 / 左内 L1 / 右内 R1 / 右外 R2 */
#define TRACK_L2_PIN         GPIO_PIN_0
#define TRACK_L2_PORT        GPIOA
#define TRACK_L1_PIN         GPIO_PIN_1
#define TRACK_L1_PORT        GPIOA
#define TRACK_R1_PIN         GPIO_PIN_2
#define TRACK_R1_PORT        GPIOA
#define TRACK_R2_PIN         GPIO_PIN_3
#define TRACK_R2_PORT        GPIOA

/* 循迹传感器压到黑线时输出低电平（看模块，如果反了把这里改成 GPIO_PIN_SET） */
#define TRACK_ON_LINE_LEVEL  GPIO_PIN_RESET

/* ---------- 运行模式 ---------- */
#define MODE_TRACKING   0   /* 自动循迹 */
#define MODE_BLUETOOTH  1   /* 蓝牙遥控 */

#define BASE_SPEED      55   /* 循迹基础速度（0~100），别调太大，容易冲出线 */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
