#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

void Error_Handler(void);

extern UART_HandleTypeDef huart2;           /* USART2 句柄 */
extern DMA_HandleTypeDef hdma_usart2_tx;    /* USART2 发送用的 DMA 通道 */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
