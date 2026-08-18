#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

void Error_Handler(void);

/* 板载 LED：PC13，低电平点亮。
 * 这里用它当「状态指示灯」：读到数据慢闪，读不到快闪。 */
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
