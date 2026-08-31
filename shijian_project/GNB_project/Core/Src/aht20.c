/* =====================================================================
 * AHT20 温湿度传感器驱动（aht20.c）
 * ---------------------------------------------------------------------
 * 读取流程（和 BMP280 完全对称，好记）：
 *   1. 上电：等 40ms，读状态字节看 bit3 是否已校准；没校准就发 0xBE 初始化
 *   2. 测量：发 0xAC 0x33 0x00 触发一次转换
 *   3. 等待：轮询状态字节的 bit7（忙标志），等它从 1 变 0（约 75ms）
 *   4. 读取：连读 6 字节（状态 + 湿度20位 + 温度20位）
 *   5. 换算：湿度 = 原始值×100/2^20，温度 = 原始值×200/2^20 - 50
 *
 * 严谨性（和 bmp280.c 同一套）：
 *   1. 全阻塞 + 全带超时，任何一步失败都返回 0，绝不卡死
 *   2. 每一步都检查 HAL 返回值
 *   3. 读数据前先等忙位清零，保证拿到的是刚测完的新数据
 *   4. 每个函数先验"初始化成功没有"，失败不算垃圾值
 *   5. 初始化成败有标志（AHT20_IsOnline 可查）
 * ===================================================================== */

#include "aht20.h"

/* ==================== 驱动内部变量（static，封装干净） ==================== */

/* 读回来的 6 字节原始数据：byte0=状态，byte1~3=湿度，byte3~5=温度 */
static uint8_t readbuffer[6];

/* 初始化成功标志：Init 成功后才置 1 */
static uint8_t aht20_online = 0;

/* ==================== 公开函数 ==================== */

/**
 * @brief  初始化传感器（含校准，开机只调一次）
 * @retval 1 = 成功；0 = 失败（芯片没接好 / 校准失败）
 */
uint8_t AHT20_Init(void)
{
    uint8_t status = 0;

    aht20_online = 0;                       /* 先假设失败 */

    /* 1. 上电后芯片内部要预热约 40ms，先等它准备好 */
    HAL_Delay(40);

    /* 2. 读状态字节，看 bit3（校准使能位）是不是 1 */
    if (HAL_I2C_Master_Receive(&hi2c1, AHT20_ADDRESS, &status, 1,
                               AHT20_TIMEOUT) != HAL_OK)
        return 0;                           /* 读都读不到，芯片没接好 */

    /* 3. 如果还没校准过（bit3=0），发 0xBE 初始化命令让它校准 */
    if ((status & 0x08) == 0)
    {
        uint8_t cmd[3] = {0xBE, 0x08, 0x00};    /* 校准命令 + 两个参数 */
        if (HAL_I2C_Master_Transmit(&hi2c1, AHT20_ADDRESS, cmd, 3,
                                    AHT20_TIMEOUT) != HAL_OK)
            return 0;

        HAL_Delay(10);                      /* 等校准完成 */

        /* 再读一次，确认 bit3 真的变成 1 了（没变 = 校准失败） */
        if (HAL_I2C_Master_Receive(&hi2c1, AHT20_ADDRESS, &status, 1,
                                   AHT20_TIMEOUT) != HAL_OK)
            return 0;
        if ((status & 0x08) == 0)
            return 0;
    }

    aht20_online = 1;                       /* 在线 */
    return 1;
}

/**
 * @brief  触发一次测量
 * @retval 1 = 命令已发出；0 = 不在线 或 发送失败
 */
uint8_t AHT20_Measure(void)
{
    static uint8_t cmd[3] = {0xAC, 0x33, 0x00};   /* 触发测量命令 + 两个参数 */

    if (aht20_online == 0) return 0;

    return HAL_I2C_Master_Transmit(&hi2c1, AHT20_ADDRESS, cmd, 3,
                                   AHT20_TIMEOUT) == HAL_OK;
}

/**
 * @brief  等测量完成 + 读出 6 字节数据（阻塞，带超时，不会死等）
 * @retval 1 = 数据已读回；0 = 不在线 / 读失败 / 转换超时
 * @note   AHT20 测一次约 75ms，期间状态字节的 bit7（忙标志）一直是 1，
 *         轮询等它变 0 才去读数据，保证拿到的是新鲜结果。
 */
uint8_t AHT20_Get(void)
{
    uint8_t  status = 0;
    uint32_t tick;

    if (aht20_online == 0) return 0;

    /* 1. 等忙位清零：bit7(0x80)=1 表示"还在测量"，一直等它变 0。
     *    用 HAL_GetTick() 做超时，最多等 AHT20_TIMEOUT 毫秒，超时返回 0。 */
    tick = HAL_GetTick();
    do
    {
        if (HAL_I2C_Master_Receive(&hi2c1, AHT20_ADDRESS, &status, 1,
                                   AHT20_TIMEOUT) != HAL_OK)
            return 0;
        if (HAL_GetTick() - tick > AHT20_TIMEOUT)
            return 0;                       /* 超时：测量一直没完成 */
    } while (status & 0x80);

    /* 2. 忙位清零了，连读 6 字节：
     *    byte0 状态 + byte1~2~3 湿度(20位) + byte3~4~5 温度(20位) */
    return HAL_I2C_Master_Receive(&hi2c1, AHT20_ADDRESS, readbuffer, 6,
                                  AHT20_TIMEOUT) == HAL_OK;
}

/**
 * @brief  把缓冲区里的原始数据换算成湿度和温度
 * @param  humidity    输出：相对湿度，单位 %（0~100）
 * @param  temperature 输出：温度，单位 ℃（-40~80）
 * @retval 1 = 成功；0 = 不在线 / 数据还没就绪
 */
uint8_t AHT20_Analysis(float *humidity, float *temperature)
{
    uint32_t rawHum, rawTemp;

    /* 防御：没初始化、或没地方放结果，直接返回失败 */
    if (aht20_online == 0) return 0;
    if (humidity == 0 && temperature == 0) return 0;

    /* 状态字节 bit7 还是 1 说明数据没就绪，别往下算 */
    if (readbuffer[0] & 0x80) return 0;

    /* 1. 拼出 20 位原始值：
     *    湿度 = byte1<<12 | byte2<<4 | byte3>>4
     *    温度 = (byte3的低4位)<<16 | byte4<<8 | byte5 */
    rawHum  = ((uint32_t)readbuffer[1] << 12) |
              ((uint32_t)readbuffer[2] << 4)  |
              ((uint32_t)readbuffer[3] >> 4);

    rawTemp = ((uint32_t)(readbuffer[3] & 0x0F) << 16) |
              ((uint32_t)readbuffer[4] << 8)  |
              (uint32_t)readbuffer[5];

    /* 2. 换算（数据手册公式）：
     *    湿度：原始值占满 20 位，所以 ÷2^20 再 ×100 就是百分比
     *    温度：原始值占满 20 位，÷2^20 再 ×200 减 50 就是 ℃ */
    if (humidity)
        *humidity    = (float)rawHum  * 100.0f / (1u << 20);
    if (temperature)
        *temperature = (float)rawTemp * 200.0f / (1u << 20) - 50.0f;

    return 1;
}

/**
 * @brief  查询传感器是否初始化成功过
 * @retval 1 = 在线；0 = 离线
 */
uint8_t AHT20_IsOnline(void)
{
    return aht20_online;
}
