#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

void Error_Handler(void);

/* ADC 句柄：由 main.c 定义，HAL 库靠它记住 ADC1 的配置 */
extern ADC_HandleTypeDef hadc1;

/* 板载 LED：PC13，低电平点亮 */
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
