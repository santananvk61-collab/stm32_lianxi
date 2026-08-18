#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

void Error_Handler(void);

/* 串口句柄：USART2 */
extern UART_HandleTypeDef huart2;

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
