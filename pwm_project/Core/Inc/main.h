#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

void Error_Handler(void);

/* 定时器句柄：TIM3（用来产生 PWM） */
extern TIM_HandleTypeDef htim3;

/* PWM 输出引脚：PA6 = TIM3_CH1（接一颗外部 LED 到 PA6） */
#define PWM_Pin GPIO_PIN_6
#define PWM_GPIO_Port GPIOA

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
