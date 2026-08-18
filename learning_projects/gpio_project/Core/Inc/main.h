#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

void Error_Handler(void);

/*
 * 板载 LED：Blue Pill 的 PC13 接了一颗 LED，
 * 而且是「低电平点亮」（写 0 亮，写 1 灭），注意别记反。
 */
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
