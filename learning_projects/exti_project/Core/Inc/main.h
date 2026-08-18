#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

void Error_Handler(void);

/* 按键：PA1，接一个按键到 GND，按下时引脚被拉低 */
#define KEY_Pin GPIO_PIN_1
#define KEY_GPIO_Port GPIOA

/* 板载 LED：PC13，低电平点亮 */
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
