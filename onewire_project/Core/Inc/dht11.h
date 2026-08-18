/*
 * dht11.h —— DHT11 温湿度传感器驱动头文件
 *
 * DHT11 用一根线（PA0）和 MCU 对话，协议是 "单总线"：
 *   - MCU 先拉低 18ms 说"我要读数了"（开始信号）
 *   - 传感器回 40 位数据：湿度高8 + 湿度低8 + 温度高8 + 温度低8 + 校验和8
 *   每一位用"高电平持续多久"来表示 0 还是 1。
 */
#ifndef __DHT11_H
#define __DHT11_H

#include "main.h"

typedef enum
{
    DHT11_OK = 0,           /* 读取成功 */
    DHT11_ERR_TIMEOUT,      /* 超时：没等到传感器应答/数据 */
    DHT11_ERR_CHECKSUM      /* 校验和不对：数据可能传错了 */
} DHT11_Status;

void DHT11_Init(void);                                       /* 初始化（开启 DWT 微秒计时） */
DHT11_Status DHT11_Read(int8_t *temperature, uint8_t *humidity);  /* 读温湿度 */

#endif /* __DHT11_H */
