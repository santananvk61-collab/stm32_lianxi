/**
  * @file    protocol.h
  * @brief   串口指令解析与波形输出接口
  */

#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include <stdint.h>

void Protocol_Init(void);
void Protocol_RxByte(uint8_t b);   /* 字节中断里逐字节喂进来 */
void Protocol_Task(void);          /* 主循环周期调用 */

#endif /* __PROTOCOL_H */
