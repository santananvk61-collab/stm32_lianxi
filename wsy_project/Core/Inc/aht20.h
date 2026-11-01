#ifndef __AHT20_H__
#define __AHT20_H__

#include <stdint.h>
#include "error_code.h"

#define AHT20_I2C_ADDR (0x38 << 1)               // AHT20 I2C地址(7位地址0x38左移1位)

DeviceError_t AHT20_Init(void);                    // 初始化AHT20,失败返回 ERR_INIT_FAILED
DeviceError_t AHT20_send(void);                    // 触发测量,失败返回 ERR_NOT_RESPOND
DeviceError_t AHT20_receive(void);                 // 读取结果,失败返回 ERR_NOT_RESPOND
int AHT20_Calc(float *temp, float *hum);           // 计算温湿度,0=有效 1=无效(保持旧语义兼容 main.c)
DeviceError_t AHT20_Read(float *temp, float *hum); // 统一接口:触发+读取+计算,返回错误码
void AHT20_GetRaw(uint32_t *raw_temp, uint32_t *raw_hum); // 获取原始码值(20位),供调试打印

#endif
