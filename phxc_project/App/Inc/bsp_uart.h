/**
  * @file    bsp_uart.h
  * @brief   USART1（调试/蓝牙）收发接口
  */

#ifndef __BSP_UART_H
#define __BSP_UART_H

#include <stdint.h>

void Uart_Init(void);
void Uart_SendString(const char *s);
void Uart_SendBuffer(const uint8_t *buf, uint16_t len);

#endif /* __BSP_UART_H */
