/**
  * @file    bsp_spi_icm42670.h
  * @brief   ICM-42670-L 寄存器定义与数据接口
  * @note   与 ICM-42688 寄存器表不同，无 BANK_SEL；配置寄存器在 0x1F/0x20/0x21。
  */

#ifndef __BSP_SPI_ICM42670_H
#define __BSP_SPI_ICM42670_H

#include <stdint.h>

typedef struct {
    float acc_g[3];      /* x,y,z 加速度，单位 g */
    float gyro_dps[3];   /* x,y,z 角速度，单位 °/s */
    float temp_c;        /* 温度，单位 ℃ */
} ICM_Data_t;

/* ---- ICM-42670 用户寄存器（bank0，直接地址访问） ---- */
#define ICM_REG_MCLK_RDY         0x00u  /* bit3=内部时钟就绪 */
#define ICM_REG_SIGNAL_RESET     0x02u  /* bit4=软复位, bit2=FIFO flush */
#define ICM_REG_TEMP_DATA_X1     0x09u  /* 温度高字节，其后 13 字节连续 */
#define ICM_REG_ACCEL_DATA_X1    0x0Bu  /* 加速度 X 高字节 */
#define ICM_REG_GYRO_DATA_X1     0x11u  /* 陀螺 X 高字节 */
#define ICM_REG_PWR_MGMT0        0x1Fu  /* bit4=IDLE, bit[3:2]=陀螺模式, bit[1:0]=加速度模式 */
#define ICM_REG_GYRO_CONFIG0     0x20u  /* bit[6:5]=量程, bit[3:0]=ODR */
#define ICM_REG_ACCEL_CONFIG0    0x21u  /* bit[6:5]=量程, bit[3:0]=ODR */
#define ICM_REG_GYRO_CONFIG1     0x23u  /* bit[2:0]=UI 滤波带宽 */
#define ICM_REG_ACCEL_CONFIG1    0x24u
#define ICM_REG_WHO_AM_I         0x75u

/* ---- 配置常量 ---- */
#define ICM_WHO_AM_I_VAL         0x67u
#define ICM_PWR_ALL_LN           0x0Fu  /* 陀螺+加速度均低噪声模式(LN=11) */
#define ICM_FS_GYRO_500DPS       0x40u  /* bit[6:5]=10 → ±500dps，65.5 LSB/(°/s) */
#define ICM_FS_ACCEL_4G          0x40u  /* bit[6:5]=10 → ±4g，8192 LSB/g */
#define ICM_ODR_1600HZ           0x05u  /* 1.6kHz：1ms 控制周期每拍都有新样本 */

uint8_t ICM_Init(void);                  /* 返回 0=成功，非 0=错误码 */
uint8_t ICM_ReadWhoAmI(void);            /* 返回 WHO_AM_I 原始值 */
uint8_t ICM_ReadData(ICM_Data_t *out);   /* 返回 0=成功 */

#endif /* __BSP_SPI_ICM42670_H */
