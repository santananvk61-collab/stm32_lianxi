#include "bmp280.h"
#include "main.h"
#include "i2c.h"

#define BMP280_I2C_ADDR (0x77 << 1)
#define BMP280_CTRL_MEAS_REG 0xF4
#define BMP280_CONFIG_REG 0xF5
#define BMP280_ID_REG 0xD0
#define BMP280_CALIB_REG 0x88
#define BMP280_REG_RESET 0xE0
#define BMP280_DATA_REG 0xF7   // 压力+温度数据起始寄存器


// 校准数据结构体（总共24个字节）
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
} bmp280_calib_data_t;

static bmp280_calib_data_t calib;

/* t_fine:温度补偿的中间值,压力补偿要用,声明成文件内 static */
static int32_t t_fine;

/* 原始数据缓存区 + 拼接后的 20 位 adc 值,文件内共享 */
static uint8_t  buf[6]      = {0};
static int32_t  adc_press   = 0;
static int32_t  adc_temp    = 0;

static HAL_StatusTypeDef BMP280_WriteReg(uint8_t *value, uint8_t len, uint16_t address)
{
    return HAL_I2C_Mem_Write(&hi2c1, BMP280_I2C_ADDR, address, I2C_MEMADD_SIZE_8BIT, value, len, 100);
}

static HAL_StatusTypeDef BMP280_ReadReg(uint8_t *data, uint8_t len, uint16_t address)
{
    return HAL_I2C_Mem_Read(&hi2c1, BMP280_I2C_ADDR, address, I2C_MEMADD_SIZE_8BIT, data, len, 100);
}

DeviceError_t BMP280_Init(void)
{
    uint8_t command = 0xB6;//复位命令字

    if(BMP280_WriteReg(&command, 1, BMP280_REG_RESET) != HAL_OK)
    {
        return ERR_INIT_FAILED;
    }
    HAL_Delay(10);

    uint8_t data;
    if(BMP280_ReadReg(&data, 1, BMP280_ID_REG) != HAL_OK)
    {
        return ERR_INIT_FAILED;
    }

    if(data != 0x58)//检查ID是否正确
    {
        return ERR_INIT_FAILED;
    }

    uint8_t calib_buf[24];
    if(BMP280_ReadReg(calib_buf, 24, BMP280_CALIB_REG) != HAL_OK)
    {
        return ERR_INIT_FAILED;
    }

    calib.dig_T1 = ((uint16_t)calib_buf[1]) << 8 | calib_buf[0];
    calib.dig_T2 = ((int16_t)calib_buf[3]) << 8 | calib_buf[2];
    calib.dig_T3 = ((int16_t)calib_buf[5]) << 8 | calib_buf[4];
    calib.dig_P1 = ((uint16_t)calib_buf[7]) << 8 | calib_buf[6];
    calib.dig_P2 = ((int16_t)calib_buf[9]) << 8 | calib_buf[8];
    calib.dig_P3 = ((int16_t)calib_buf[11]) << 8 | calib_buf[10];
    calib.dig_P4 = ((int16_t)calib_buf[13]) << 8 | calib_buf[12];
    calib.dig_P5 = ((int16_t)calib_buf[15]) << 8 | calib_buf[14];
    calib.dig_P6 = ((int16_t)calib_buf[17]) << 8 | calib_buf[16];
    calib.dig_P7 = ((int16_t)calib_buf[19]) << 8 | calib_buf[18];
    calib.dig_P8 = ((int16_t)calib_buf[21]) << 8 | calib_buf[20];
    calib.dig_P9 = ((int16_t)calib_buf[23]) << 8 | calib_buf[22];

    // ctrl_meas 值:101_101_01 = 0xB5
    uint8_t ctrl = 0xB5;   // 或 0b10110101

    // config 值:000_101_0_0 = 0x14 (bit[1] 保留=0)
    uint8_t config = 0x14; // 或 0b00010100

    if(BMP280_WriteReg(&config, 1, BMP280_CONFIG_REG) != HAL_OK)
    {
        return ERR_INIT_FAILED;
    }
    if(BMP280_WriteReg(&ctrl,   1, BMP280_CTRL_MEAS_REG) != HAL_OK)
    {
        return ERR_INIT_FAILED;
    }

    HAL_Delay(20);

    return ERR_OK;
}

/* 触发一次测量(Forced 模式下必须先调用这个,再读数据)
   bit[7:5]=101 温度过采样 x16
   bit[4:2]=101 压力过采样 x16
   bit[1:0]=01  Forced 模式(测完自动回 Sleep) */
DeviceError_t BMP280_TriggerMeasure(void)
{
    uint8_t ctrl = 0xB5;   // 0b10110101
    if(BMP280_WriteReg(&ctrl, 1, BMP280_CTRL_MEAS_REG) != HAL_OK)
    {
        return ERR_NOT_RESPOND;
    }
    return ERR_OK;
}

/* 读取 6 字节原始数据到 buf
   datasheet 4.3 节:从 0xF7 起连续读 press(3) + temp(3)
   返回值: ERR_OK 成功 */
DeviceError_t BMP280_ReadRaw(void)
{
    if(BMP280_ReadReg(buf, 6, BMP280_DATA_REG) != HAL_OK)
    {
        return ERR_NOT_RESPOND;
    }
    return ERR_OK;
}

/* 把 buf 的 6 字节拼接成 20 位 adc 值
   datasheet 4.3 节:
     adc_press = press_msb<<12 | press_lsb<<4 | press_xlsb>>4
     adc_temp  = temp_msb<<12  | temp_lsb<<4  | temp_xlsb>>4
   注意:xlsb 的低 4 位是 0,datasheet 说是 "4.2 LSB",丢弃
   前置条件:必须先成功调用 BMP280_ReadRaw() */
static void BMP280_Pack(void)
{
    /* 拼接 20 位压力原始值
       buf[0]=press_msb, buf[1]=press_lsb, buf[2]=press_xlsb(高4位有效) */
    adc_press = ((int32_t)buf[0] << 12)
              | ((int32_t)buf[1] << 4)
              | ((int32_t)buf[2] >> 4);

    /* 拼接 20 位温度原始值
       buf[3]=temp_msb, buf[4]=temp_lsb, buf[5]=temp_xlsb(高4位有效) */
    adc_temp = ((int32_t)buf[3] << 12)
              | ((int32_t)buf[4] << 4)
              | ((int32_t)buf[5] >> 4);
}

/* 补偿计算:把 adc 原始值换成物理量
   P:输出补偿后的压力(Pa)
   T:输出补偿后的温度(℃)
   注意:温度必须先算,得到 t_fine 后才能算压力*/
void BMP280_Compensate(double *P, double *T)
{
    double var1, var2;

    /* === 温度补偿(datasheet 4.3.1) ===
       4 个常量都是 2 的整数次幂:
         16384 = 2^14, 1024 = 2^10, 131072 = 2^17, 8192 = 2^13 */
    var1 = ((double)adc_temp / 16384.0 - (double)calib.dig_T1 / 1024.0) * calib.dig_T2;
    var2 = ((double)adc_temp / 131072.0 - (double)calib.dig_T1 / 8192.0)
         * ((double)adc_temp / 131072.0 - (double)calib.dig_T1 / 8192.0)
         * calib.dig_T3;
    t_fine = (int32_t)(var1 + var2);                    /* 保存中间值,压力计算要用 */
    *T = (double)t_fine / 5120.0;                       /* 换算为摄氏度 */

    /* === 压力补偿(datasheet 4.3.1) ===
       注意:dig_P10 是 datasheet 笔误,实际是常数 -7 */
    var1 = ((double)t_fine / 2.0) - 64000.0;
    var2 = var1 * var1 * (double)calib.dig_P6 / 32768.0;
    var2 = var2 + var1 * (double)calib.dig_P5 * 2.0;
    var2 = var2 / 4.0 + (double)calib.dig_P4 * 65536.0;
    var1 = ((double)calib.dig_P3 * var1 * var1 / 524288.0
          + (double)calib.dig_P2 * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * (double)calib.dig_P1;

    double p = 1048576.0 - (double)adc_press;
    p = (p - var2 / 4096.0) * 6250.0 / var1;
    var1 = (double)calib.dig_P9 * p * p / 2147483648.0;
    var2 = -7.0 * p / 32768.0;                          /* dig_P10 替换为常数 -7 */
    p = p + (var1 + var2 + 3791.0) / 16.0;
    *P = p;

}

DeviceError_t BMP280_Read(double *P, double *T)
{
    DeviceError_t err;

    err = BMP280_TriggerMeasure();
    if(err != ERR_OK) return err;

    HAL_Delay(80);

    err = BMP280_ReadRaw();
    if(err != ERR_OK) return err;

    BMP280_Pack();

    BMP280_Compensate(P, T);

    return ERR_OK;
}

/* 获取原始码值(20位有符号整数),供串口调试打印用
   Read 成功后 adc_press/adc_temp 已在 Pack() 里更新 */
void BMP280_GetRaw(int32_t *raw_press, int32_t *raw_temp)
{
    *raw_press = adc_press;
    *raw_temp  = adc_temp;
}
