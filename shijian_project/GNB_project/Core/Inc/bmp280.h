#ifndef __BMP280_H__
#define __BMP280_H__

#include "i2c.h"        // 为了拿到 hi2c1（I2C1 的句柄）
#include "main.h"
#include <stdint.h>

/* =====================================================================
 * BMP280 气压/温度传感器驱动（bmp280.h）
 * ---------------------------------------------------------------------
 * 和 AHT20 挂在同一条 I2C1 总线上。AHT20 量温湿度，BMP280 量气压+温度。
 *
 * 四个函数，全阻塞、全带超时，每个都返回"成功(1) / 失败(0)"：
 *   BMP280_Init()     开机调一次，读校准系数
 *   BMP280_Measure()  触发一次测量
 *   BMP280_Get()      等转换完成 + 读出原始数据（内部会自动等状态位）
 *   BMP280_Analysis() 把原始值换算成 hPa 和 ℃
 *
 * 为什么全用阻塞（不用中断）？
 *   一次 I2C 读写只要零点几毫秒，真正耗时的是芯片转换（约 41ms），
 *   而等待是靠"轮询状态位"实现的。所以中断在这里省不下时间，
 *   反而要多写两个回调、多一层状态机，更容易出错。
 *   阻塞 + 带超时 = 最简单，也最不容易卡死。
 * ===================================================================== */

/* ==================== I2C 地址 ====================
 * 注意：这里填 7 位地址（0x76），驱动调用 HAL 时才会自动左移 1 位。
 * 模块上 SDO 接地 -> 0x76（大多数）；SDO 接 VCC -> 0x77。 */
#define BMP280_ADDRESS          0x77
#define BMP280_I2C_ADDR_0x77    0x77

/* ==================== 寄存器地址 ====================
 * 寄存器 = 芯片内部带编号的小格子，往里写 = 下命令，往外读 = 拿结果 */
#define BMP280_REG_CALIB        0x88   /* 校准系数起始地址（连读 24 字节） */
#define BMP280_REG_CHIP_ID      0xD0   /* 芯片身份证，读出来固定是 0x58 */
#define BMP280_REG_RESET        0xE0   /* 软复位，写入 0xB6 芯片重启 */
#define BMP280_REG_STATUS       0xF3   /* 状态：bit3=正在测量，bit0=正在搬校准值 */
#define BMP280_REG_CTRL_MEAS    0xF4   /* 测量控制：过采样倍数 + 工作模式 */
#define BMP280_REG_CONFIG       0xF5   /* 配置：待机时间 + 滤波 */
#define BMP280_REG_PRESS_MSB    0xF7   /* 从这里连读 6 字节：气压3字节 + 温度3字节 */

/* ==================== 常用值 ==================== */
#define BMP280_CHIP_ID_VALUE    0x58   /* 芯片 ID 的固定值 */
#define BMP280_RESET_VALUE      0xB6   /* 软复位的"暗号" */

/* 写进 CTRL_MEAS(0xF4) 的命令字，拆成三段看：
 *   bit7~5 = 温度过采样倍数    bit4~2 = 气压过采样倍数    bit1~0 = 工作模式
 *   0x34 = 001 101 00 ：温度×1 + 气压×16 + 睡眠（只配置，不测量）
 *   0x35 = 001 101 01 ：温度×1 + 气压×16 + forced 单次测量（测完自动睡）
 * 想让转换快一点就把气压改成 ×1：0x24(睡眠) / 0x25(单次)，
 * 转换耗时从 41ms 降到 6ms，代价是噪声大一些。 */
#define BMP280_CTRL_SLEEP       0x34
#define BMP280_CTRL_FORCED      0x35

/* 写进 CONFIG(0xF5) 的配置字
 *   0x10 = 待机 0.5ms(000) + 滤波系数×4(100)，滤波能让数值更稳不容易跳 */
#define BMP280_CONFIG_VAL       0x10

/* I2C 单次操作的超时时间（毫秒）。
 * 带超时意味着：芯片没应答/总线卡住时，函数最多等这么久就返回失败，
 * 绝不会死等。之前那个 while 死循环就是栽在"没有超时"上。 */
#define BMP280_I2C_TIMEOUT      100

/* ==================== 校准数据结构体 ====================
 * 这 12 个数是每颗芯片出厂时逐个测好写进去的"体检报告"，
 * 温度用 T1~T3，气压用 P1~P9，换算时缺一不可。 */
typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
} BMP280_Calib_t;

/* ==================== 设备结构体 ====================
 * 把"用哪个 I2C 口 + 芯片地址 + 校准系数 + 中间量"打包在一起管理 */
typedef struct {
    I2C_HandleTypeDef *hi2c;   /* I2C 句柄指针（告诉驱动用哪个 I2C 口） */
    uint8_t addr;              /* 7 位地址（0x76 或 0x77） */
    BMP280_Calib_t calib;      /* 校准系数 */
    int32_t t_fine;            /* 温度补偿的中间值，算气压时必须接着用 */
} BMP280_t;

/* ==================== 对外函数 ====================
 * 全部返回 uint8_t：1 = 成功，0 = 失败。
 * 每个函数内部都先检查"传感器有没有初始化成功"，没初始化就直接返回 0，
 * 不会拿全 0 的校准系数算出一堆垃圾。 */

uint8_t BMP280_Init(void);      /* 初始化：验ID + 软复位 + 读校准系数 + 配置。成功返回1 */
uint8_t BMP280_Measure(void);   /* 触发一次测量。命令发出成功返回1 */
uint8_t BMP280_Get(void);       /* 等转换完成(查状态位) + 读6字节。成功返回1 */
uint8_t BMP280_Analysis(float *pressure, float *temperature); /* 换算：pressure=hPa，temperature=℃。成功返回1 */
uint8_t BMP280_Read_ID(void);   /* 读芯片ID（调试用，正常返回0x58，失败返回0） */
uint8_t BMP280_IsOnline(void);  /* 查询：初始化是否成功过（1=在线，0=离线） */

/* 状态机变量（和 aht20.h 里的 aht20State 一个套路，定义在 main.c） */
extern uint8_t bmp280State;

#endif /* __BMP280_H__ */
