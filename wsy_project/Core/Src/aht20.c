#include "aht20.h"
#include "i2c.h"



static uint8_t aht20_data[6] = {0};//存放AHT20返回的数据
static uint8_t data[3] = {0xAC, 0x33, 0x00};//发送数据指令


DeviceError_t AHT20_Init(void)
{
    HAL_Delay(40);

    uint8_t cmd = 0x71;
    uint8_t status = 0;
    if(HAL_I2C_Master_Transmit(&hi2c1, AHT20_I2C_ADDR, &cmd, 1, 100) != HAL_OK) return ERR_INIT_FAILED;
    if(HAL_I2C_Master_Receive(&hi2c1, AHT20_I2C_ADDR, &status, 1, 100) != HAL_OK) return ERR_INIT_FAILED;

    if((status & (1 << 3)) == 0)
    {
        uint8_t init_cmd[ 3 ] = { 0xBE , 0x08 , 0x00 };
        if(HAL_I2C_Master_Transmit(&hi2c1, AHT20_I2C_ADDR, init_cmd, 3, 100) != HAL_OK) return ERR_INIT_FAILED;
        HAL_Delay(20);      
    }
    return ERR_OK;
}

/* 触发测量:发送 0xAC 0x33 0x00 三个字节(阻塞式)
   超时 100ms,I2C 总线异常时不会永久卡死 */
DeviceError_t AHT20_send(void)
{
    if(HAL_I2C_Master_Transmit(&hi2c1, AHT20_I2C_ADDR, data, 3, 100) != HAL_OK) return ERR_NOT_RESPOND;
    return ERR_OK;
}

/* 读取测量结果:接收 6 字节(状态+湿度20位+温度20位),阻塞式
   超时 100ms */
DeviceError_t AHT20_receive(void)
{
    if(HAL_I2C_Master_Receive(&hi2c1, AHT20_I2C_ADDR, aht20_data, 6, 100) != HAL_OK) return ERR_NOT_RESPOND;
    return ERR_OK;
}

/* 内部函数:把 aht20_data 的 [1][2][3] 三字节拼成 20 位湿度原始值
   datasheet:湿度 = data[1]<<12 | data[2]<<4 | data[3]>>4 */
static uint32_t AHT20_PackHumidity(const uint8_t *buf)
{
    return ((uint32_t)buf[1] << 12)
         | ((uint32_t)buf[2] << 4)
         | ((uint32_t)buf[3] >> 4);
}

/* 内部函数:把 aht20_data 的 [3][4][5] 三字节拼成 20 位温度原始值
   datasheet:温度 = (data[3]&0x0F)<<16 | data[4]<<8 | data[5]
   注意 data[3] 是共享字节,高4位属于湿度,低4位属于温度 */
static uint32_t AHT20_PackTemperature(const uint8_t *buf)
{
    return ((uint32_t)(buf[3] & 0x0F) << 16)
         | ((uint32_t)buf[4] << 8)
         |  (uint32_t)buf[5];
}

// 返回值: 1=数据有效, 0=数据无效(BUSY未完成)
// temp: 输出温度(℃), hum: 输出湿度(%)
int AHT20_Calc(float *temp, float *hum)
{
    /* 阻塞式下 aht20_data 只在主循环读写,不会被中断打断,不需要快照 */

    /* 1. 检查状态字节 bit7(BUSY),为1说明测量未完成,数据无效 */
    if ((aht20_data[ 0 ] & ( 1 << 7 )) || (aht20_data[ 0 ] & ( 1 << 3 )) == 0 ) return 1 ;

    /* 2. 拼接原始值并换算为物理量
          湿度 = raw * 100 / 2^20
          温度 = raw * 200 / 2^20 - 50
          2^20 = 1048576 */
    uint32_t raw_hum  = AHT20_PackHumidity(aht20_data);
    uint32_t raw_temp = AHT20_PackTemperature(aht20_data);

    *hum  = (float)raw_hum  * 100.0f / 1048576.0f;
    *temp = (float)raw_temp * 200.0f / 1048576.0f - 50.0f;

    return 0;
}

/* 统一读接口:依次执行 触发->延时80ms->读取->计算
   任一步失败立即返回对应错误码,不再继续往下走
   供后续主循环/状态机统一调用,迁移时可用它替代分散调用 */
DeviceError_t AHT20_Read(float *temp, float *hum)
{
    DeviceError_t err;

    err = AHT20_send();
    if(err != ERR_OK) return err;

    HAL_Delay(80);

    err = AHT20_receive();
    if(err != ERR_OK) return err;

    if(AHT20_Calc(temp, hum) != 0) return ERR_INVALID_DATA;

    return ERR_OK;
}

/* 获取原始码值(20位整数),供串口调试打印用
   从 aht20_data 重新拼接,Read 成功后 aht20_data 已更新 */
void AHT20_GetRaw(uint32_t *raw_temp, uint32_t *raw_hum)
{
    *raw_hum  = AHT20_PackHumidity(aht20_data);
    *raw_temp = AHT20_PackTemperature(aht20_data);
}
