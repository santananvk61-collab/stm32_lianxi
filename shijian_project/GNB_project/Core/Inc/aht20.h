#ifndef INC_AHT20_H_
#define INC_AHT20_H_

#include "i2c.h"
#include "main.h"
#include <stdint.h>

/* =====================================================================
 * AHT20 温湿度传感器驱动（aht20.h）
 * ---------------------------------------------------------------------
 * 和 BMP280 挂在同一条 I2C1 总线上。AHT20 量温湿度，BMP280 量气压+温度。
 *
 * 四个函数，全阻塞、全带超时，每个都返回"成功(1) / 失败(0)"，
 * 和 bmp280.h 的套路完全一致：
 *   AHT20_Init()     开机调一次（含校准）
 *   AHT20_Measure()  触发一次测量
 *   AHT20_Get()      等测量完成 + 读出 6 字节
 *   AHT20_Analysis() 换算成湿度和温度
 * ===================================================================== */

/* I2C 地址：AHT20 的 7 位地址是 0x38，HAL 要求填"左移 1 位"后的 0x70。
 * 读的时候硬件会在末尾自动补上"读"位变成 0x71，我们不用管。 */
#define AHT20_ADDRESS  0x70

/* I2C 单次操作的超时时间（毫秒）。
 * 芯片没应答/总线卡住时，函数最多等这么久就返回失败，绝不会卡死。
 * （旧代码用 HAL_MAX_DELAY = 死等，是卡死的根源） */
#define AHT20_TIMEOUT  100

/* 对外函数：全部返回 1 = 成功，0 = 失败 */
uint8_t AHT20_Init(void);      /* 初始化 + 校准。成功返回1 */
uint8_t AHT20_Measure(void);   /* 触发测量。命令发出成功返回1 */
uint8_t AHT20_Get(void);       /* 等测量完成(查忙位) + 读6字节。成功返回1 */
uint8_t AHT20_Analysis(float *humidity, float *temperature); /* 换算。成功返回1 */
uint8_t AHT20_IsOnline(void);  /* 查询：初始化是否成功过（1=在线 0=离线） */

/* 状态机变量（定义在 main.c。改成阻塞版后其实用不到了，先留着兼容） */
extern uint8_t aht20State;

#endif /* INC_AHT20_H_ */
