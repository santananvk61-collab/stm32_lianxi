// aht20传感器读取流程
// 1.上电后要等待40ms，读取温湿度值之前，首先要看状态字的校准使能位Bit[3]是否为1
// (通过发送0x71可以获取一个字节的状态字)，如果不为1，要发送0xBE命令(初始化)，
// 此命令参数有两个字节，第一个字节为0x08，第二个字节为0x00。
// 2.直接发送0xAC命令(触发测量)，此命令参数有两个字节，第一个字节为0x33，第二个字节为0x00。
// 3.发OX71开始读取，等待75ms待测量完成，忙状态Bit[7]为0，然后可以读取六个字节。
// 4.计算温湿度值。

#include "aht20.h"

#define AHT20_ADDRESS 0x70

uint8_t readbuffer[6];

//启动函数，为1正常启动，不为1初始化后启动
void AHT20_Init()
{
    uint8_t status;
    HAL_Delay(40);
    //此函数为读取函数，所有0x70会被HAL库自动改为0x71使用
    HAL_I2C_Master_Receive(&hi2c1, AHT20_ADDRESS, &status, 1,HAL_MAX_DELAY);
    //计算BIT[3]是否为1(结果为0x08)，为0（结果为0x00）则初始化
    if ((status & 0x08) == 0x00)
    {
        uint8_t sendBuffer[3] = {0xBE,0x08,0x00};
        HAL_I2C_Master_Transmit(&hi2c1, AHT20_ADDRESS, sendBuffer, 3,HAL_MAX_DELAY);
    }
    
}

void AHT20_Measure()//发送测量指令
{
    static uint8_t sendBuffer[3] = {0xAC,0x33,0x00};//开始测量指令
    //发送函数，地址与开始测量指令均发送
    HAL_I2C_Master_Transmit_IT(&hi2c1, AHT20_ADDRESS, sendBuffer, 3);
}

void AHT20_Get()//读取测量数据
{
    //读取发回来的6字节数据，存在readbuffer
    HAL_I2C_Master_Receive_IT(&hi2c1, AHT20_ADDRESS, readbuffer, 6);
}

//计算测量数据并传出去
void AHT20_Analysis(float *humidity, float *temperature)
{
    //检测第0字节的第7位是否为0，0（0x00）正常，1（0x80）表示忙/异常
    if ((readbuffer[0] & 0x80) == 0x00)
    {
        uint32_t rawHum, rawTemp;
        //湿度：20位 = buf[1]<<12 | buf[2]<<4 | buf[3]>>4
        rawHum = ((uint32_t)readbuffer[1] << 12) |
                 ((uint32_t)readbuffer[2] << 4)  |
                 ((uint32_t)readbuffer[3] >> 4);
        //温度：20位 = (buf[3]&0x0F)<<16 | buf[4]<<8 | buf[5]
        rawTemp = ((uint32_t)(readbuffer[3] & 0x0F) << 16) |
                  ((uint32_t)readbuffer[4] << 8)  |
                  (uint32_t)readbuffer[5];

        //AHT20 湿度公式：0~100%；温度公式：-50~100℃
        *humidity    = (float)rawHum  * 100.0f / (1u << 20);
        *temperature = (float)rawTemp * 200.0f / (1u << 20) - 50.0f;
    }
}
