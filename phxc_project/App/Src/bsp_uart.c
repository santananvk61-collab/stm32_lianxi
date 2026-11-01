/**
  * @file    bsp_uart.c
  * @brief   USART1（PA9/PA10，115200 8N1）调试/蓝牙口
  * @note   RX 中断逐字节喂协议层并重挂收；TX 阻塞式，仅主循环使用。
  */

#include <string.h>
#include "bsp_uart.h"
#include "usart.h"
#include "protocol.h"

#define UART_TX_TIMEOUT_MS  100u

static uint8_t s_rx_byte;

/* 挂载首字节中断接收 */
void Uart_Init(void)
{
    (void)HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1u);
}

/* 阻塞发送字符串 */
void Uart_SendString(const char *s)
{
    if (s == 0) {
        return;
    }
    (void)HAL_UART_Transmit(&huart1, (const uint8_t *)s, (uint16_t)strlen(s), UART_TX_TIMEOUT_MS);
}

/* 阻塞发送指定长度缓冲 */
void Uart_SendBuffer(const uint8_t *buf, uint16_t len)
{
    if (buf == 0 || len == 0u) {
        return;
    }
    (void)HAL_UART_Transmit(&huart1, buf, len, UART_TX_TIMEOUT_MS);
}

/* RX 完成回调：喂协议层后重挂收 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        Protocol_RxByte(s_rx_byte);
        (void)HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1u);
    }
}

/* 噪声/溢出等错误后重新挂收，避免 ORE 导致永久停止接收 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        (void)HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1u);
    }
}

/* printf 重定向（syscalls.c 中 _write 调用本 weak 符号的强实现） */
int __io_putchar(int ch)
{
    uint8_t c = (uint8_t)ch;
    (void)HAL_UART_Transmit(&huart1, &c, 1u, 10u);
    return ch;
}
