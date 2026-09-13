#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#include "main.h"

/* 蓝牙遥控模块：HC-05 通过 USART1 收手机指令，解析成电机动作 */

extern volatile uint8_t bt_flag;   /* 收到新指令标志 */
extern volatile uint8_t bt_cmd;    /* 收到的指令字符 */

void Bluetooth_Init(void);          /* 开启串口接收中断 */
void Bluetooth_Process(void);       /* 解析指令并执行动作 */

#endif
