/**
  * @file    bsp_spi_icm42670.c
  * @brief   ICM-42670-L（SPI1，软件 CS=PA4）
  * @note   Mode0/MSB/5.25MHz。读时序首字节 [reg|0x80] 无效，第 2 字节起为数据。
  */

#include <string.h>
#include "bsp_spi_icm42670.h"
#include "main.h"
#include "spi.h"

/* 量程换算系数（datasheet 电气特性表） */
#define ICM_ACC_SCALE       8192.0f    /* ±4g 时 LSB/g */
#define ICM_GYRO_SCALE      65.5f      /* ±500dps 时 LSB/(°/s) */
#define ICM_TEMP_SCALE      128.0f     /* 温度 LSB/℃，零点 25℃ */

#define ICM_SPI_TIMEOUT_MS  10u

static void icm_cs_low(void)
{
    HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_RESET);
}

static void icm_cs_high(void)
{
    HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_SET);
}

/* 读单个寄存器：发 [reg|0x80] [dummy]，数据在第 2 个返回字节 */
static uint8_t icm_reg_read(uint8_t reg)
{
    uint8_t tx[2];
    uint8_t rx[2];

    tx[0] = (uint8_t)(reg | 0x80u);
    tx[1] = 0x00u;

    icm_cs_low();
    (void)HAL_SPI_TransmitReceive(&hspi1, tx, rx, 2, ICM_SPI_TIMEOUT_MS);
    icm_cs_high();

    return rx[1];
}

/* 写单个寄存器：写时序 bit7=0 */
static void icm_reg_write(uint8_t reg, uint8_t val)
{
    uint8_t tx[2];

    tx[0] = reg & 0x7Fu;
    tx[1] = val;

    icm_cs_low();
    (void)HAL_SPI_TransmitReceive(&hspi1, tx, 0, 2, ICM_SPI_TIMEOUT_MS);
    icm_cs_high();
}

/* ICM_ReadWhoAmI：读 WHO_AM_I(0x75) 原始值（期望 0x67）。 */
uint8_t ICM_ReadWhoAmI(void)
{
    return icm_reg_read(ICM_REG_WHO_AM_I);
}

/* ICM_Init：校验 WHO_AM_I -> 软复位 -> 等待内部时钟 -> 配置量程/ODR -> 开启 LN 模式。
 * 返回 0=成功，1=ID 错误，2=内部时钟未就绪，3=SPI 通信异常。 */
uint8_t ICM_Init(void)
{
    uint32_t tick_start;

    /* CS 默认拉高，上电后等待传感器内部稳定 */
    icm_cs_high();
    HAL_Delay(2);

    if (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY) {
        return 3u;
    }
    if (ICM_ReadWhoAmI() != ICM_WHO_AM_I_VAL) {
        return 1u;
    }

    /* 清空 FIFO（本工程不用 FIFO，保险起见清一次） */
    icm_reg_write(ICM_REG_SIGNAL_RESET, 0x04u);
    HAL_Delay(1);

    /* 信号路径软复位（复位全部配置寄存器） */
    icm_reg_write(ICM_REG_SIGNAL_RESET, 0x10u);
    HAL_Delay(10);

    /* 等待内部 RC 时钟就绪（MCLK_RDY bit3），最多等 50ms */
    tick_start = HAL_GetTick();
    while ((icm_reg_read(ICM_REG_MCLK_RDY) & 0x08u) == 0u) {
        if ((HAL_GetTick() - tick_start) > 50u) {
            return 2u;
        }
    }

    /* 量程 + ODR：陀螺 ±500dps@1.6kHz，加速度 ±4g@1.6kHz */
    icm_reg_write(ICM_REG_GYRO_CONFIG0, (uint8_t)(ICM_FS_GYRO_500DPS | ICM_ODR_1600HZ));
    icm_reg_write(ICM_REG_ACCEL_CONFIG0, (uint8_t)(ICM_FS_ACCEL_4G | ICM_ODR_1600HZ));

    /* UI 滤波保持复位默认（BW=ODR/2=800Hz）；S1 试车如需降噪可改 CONFIG1 位[2:0] */
    icm_reg_write(ICM_REG_GYRO_CONFIG1, 0x00u);
    icm_reg_write(ICM_REG_ACCEL_CONFIG1, 0x00u);

    /* 陀螺/加速度均进入低噪声模式；模式切换后内部需要约 200us，留足启动时间 */
    icm_reg_write(ICM_REG_PWR_MGMT0, ICM_PWR_ALL_LN);
    HAL_Delay(10);

    return 0u;
}

/* ICM_ReadData：突发读取 0x09 起 14 字节（温度2 + 加速度6 + 陀螺6，大端序）。
 * 返回 0=成功。 */
uint8_t ICM_ReadData(ICM_Data_t *out)
{
    uint8_t tx[15];
    uint8_t rx[15];
    int16_t raw_t;
    int16_t raw_a[3];
    int16_t raw_g[3];
    HAL_StatusTypeDef st;

    if (out == 0) {
        return 1u;
    }

    memset(tx, 0, sizeof(tx));
    tx[0] = (uint8_t)(ICM_REG_TEMP_DATA_X1 | 0x80u);

    icm_cs_low();
    st = HAL_SPI_TransmitReceive(&hspi1, tx, rx, 15, ICM_SPI_TIMEOUT_MS);
    icm_cs_high();

    if (st != HAL_OK) {
        return 2u;
    }

    /* rx[0] 无效；rx[1..14] 依次为 T/T、AX/AX、AY/AY、AZ/AZ、GX/GX、GY/GY、GZ/GZ */
    raw_t   = (int16_t)(((uint16_t)rx[1] << 8) | rx[2]);
    raw_a[0] = (int16_t)(((uint16_t)rx[3] << 8) | rx[4]);
    raw_a[1] = (int16_t)(((uint16_t)rx[5] << 8) | rx[6]);
    raw_a[2] = (int16_t)(((uint16_t)rx[7] << 8) | rx[8]);
    raw_g[0] = (int16_t)(((uint16_t)rx[9] << 8) | rx[10]);
    raw_g[1] = (int16_t)(((uint16_t)rx[11] << 8) | rx[12]);
    raw_g[2] = (int16_t)(((uint16_t)rx[13] << 8) | rx[14]);

    out->temp_c = (float)raw_t / ICM_TEMP_SCALE + 25.0f;
    out->acc_g[0] = (float)raw_a[0] / ICM_ACC_SCALE;
    out->acc_g[1] = (float)raw_a[1] / ICM_ACC_SCALE;
    out->acc_g[2] = (float)raw_a[2] / ICM_ACC_SCALE;
    out->gyro_dps[0] = (float)raw_g[0] / ICM_GYRO_SCALE;
    out->gyro_dps[1] = (float)raw_g[1] / ICM_GYRO_SCALE;
    out->gyro_dps[2] = (float)raw_g[2] / ICM_GYRO_SCALE;

    return 0u;
}
