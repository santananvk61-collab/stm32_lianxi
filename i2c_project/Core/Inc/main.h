#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

void Error_Handler(void);

/* I2C1 句柄：ssd1306.c 这个驱动文件要用它跟 OLED 通信 */
extern I2C_HandleTypeDef hi2c1;

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
