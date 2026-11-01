#ifndef __BMP280_H__

#define __BMP280_H__

#include <stdint.h>
#include "error_code.h"

DeviceError_t BMP280_Init(void);                   // 初始化 BMP280
DeviceError_t BMP280_TriggerMeasure(void);         // 触发 BMP280 测量
DeviceError_t BMP280_ReadRaw(void);                //读取 BMP280 原始数据
void BMP280_Compensate(double *P, double *T);       //计算温度和压力
DeviceError_t BMP280_Read(double *P, double *T);  // 统一接口:触发+读取+计算,返回错误码
void BMP280_GetRaw(int32_t *raw_press, int32_t *raw_temp); // 获取原始码值(20位),供调试打印

#endif