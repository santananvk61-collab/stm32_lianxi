/* =====================================================================
 * BMP280 气压/温度传感器驱动（bmp280.c）
 * ---------------------------------------------------------------------
 * 读取流程（和 AHT20 完全对称，好记）：
 *   1. 上电：读 0x88 开始的 24 字节校准系数（每颗芯片都不一样，必须读）
 *   2. 测量：往 0xF4 写 0x35 触发一次转换（forced 模式，测完自动休眠）
 *   3. 等待：查状态寄存器 0xF3 的 bit3，等它从"测量中"变成"测完了"
 *   4. 读取：从 0xF7 连读 6 字节（前 3 字节气压，后 3 字节温度）
 *   5. 换算：用校准系数把原始值算成真实的 hPa 和 ℃
 *
 * 严谨性设计（本次重写的重点）：
 *   1. 全阻塞 + 全带超时（100ms），任何一步失败都会返回 0，绝不卡死
 *   2. 每一步都检查 HAL 返回值，不检查就等于是"假装成功了"
 *   3. 读数据前先查状态位，保证拿到的是"刚测完的新数据"而不是旧数据
 *   4. 每个函数都先验"初始化成功没有"，没成功就返回 0，不算垃圾值
 *   5. 初始化成败有明确标志（BMP280_IsOnline 可查）
 *
 * 为什么必须读校准系数？
 *   芯片里那 20 位原始值只是"ADC 数出来的格子数"，同样的气压，
 *   每颗芯片数出来的数都不一样。校准系数就是厂家在工厂里逐颗测好的
 *   换算参数，缺了它算出来的数能差出十万八千里。
 * ===================================================================== */

#include "bmp280.h"
#include "main.h"   // 这里包含了 HAL 库和 hi2c1 等外设声明

/* ==================== 驱动内部变量（static，外部看不见，封装更干净） ==================== */

/* 设备结构体。只有一个传感器，所以定义成静态全局的，外面不用管 */
static BMP280_t bmp280_dev;

/* 读回来的 6 字节原始数据（气压MSB+LSB+XLSB，温度MSB+LSB+XLSB） */
static uint8_t bmp280_rx_buffer[6] = {0};

/* 初始化成功标志：Init 成功后才置 1，其它函数都靠它判断"传感器在不在线" */
static uint8_t bmp280_online = 0;

/* ==================== 底层 I2C 读写封装（全阻塞 + 带超时，只给驱动内部用） ====================
 * 三个函数都返回 1=成功 0=失败，且都带 BMP280_I2C_TIMEOUT(100ms) 超时。
 * 注意：超时是"出错保护"，正常通信一次只要零点几毫秒，不会真的等 100ms。 */

/* 往 reg 号寄存器写 1 个字节（下命令） */
static uint8_t BMP280_WriteReg(BMP280_t *dev, uint8_t reg, uint8_t data)
{
    /* dev->addr << 1：HAL 要的是"7位地址左移1位"的 8 位格式 */
    return HAL_I2C_Mem_Write(dev->hi2c, dev->addr << 1, reg,
                             I2C_MEMADD_SIZE_8BIT, &data, 1,
                             BMP280_I2C_TIMEOUT) == HAL_OK;
}

/* 从 reg 号寄存器读 1 个字节 */
static uint8_t BMP280_ReadReg(BMP280_t *dev, uint8_t reg, uint8_t *data)
{
    return HAL_I2C_Mem_Read(dev->hi2c, dev->addr << 1, reg,
                            I2C_MEMADD_SIZE_8BIT, data, 1,
                            BMP280_I2C_TIMEOUT) == HAL_OK;
}

/* 从 reg 号寄存器开始连读 len 个字节。
 * 连读很重要：气压+温度共 6 字节必须一口气读完，
 * 分两次读的话数据可能正好跨在两次测量之间，拼出个错值。 */
static uint8_t BMP280_ReadRegs(BMP280_t *dev, uint8_t reg, uint8_t *buf, uint8_t len)
{
    return HAL_I2C_Mem_Read(dev->hi2c, dev->addr << 1, reg,
                            I2C_MEMADD_SIZE_8BIT, buf, len,
                            BMP280_I2C_TIMEOUT) == HAL_OK;
}

/* ==================== 公开函数 ==================== */

/**
 * @brief  初始化传感器（阻塞模式，开机只调一次）
 * @retval 1 = 成功；0 = 失败（芯片没接好 / 地址不对 / 总线异常）
 * @note   失败不会卡死，也不会往下走，bmp280_online 保持 0，
 *         后面所有函数都会因此直接返回 0，不会拿垃圾数据计算。
 */
uint8_t BMP280_Init(void)
{
    uint8_t id = 0;
    uint8_t calib_buf[24] = {0};

    bmp280_online = 0;                      /* 先假设失败，全部通过再置 1 */

    /* 1. 记下用哪个 I2C 口、芯片地址是多少 */
    bmp280_dev.hi2c = &hi2c1;
    bmp280_dev.addr = BMP280_ADDRESS;

    /* 2. 验芯片身份证：0xD0 读出来必须是 0x58。
     *    读失败（返回值 0）或读出来的值不对，都算失败，直接返回。 */
    if (BMP280_ReadReg(&bmp280_dev, BMP280_REG_CHIP_ID, &id) == 0)
        return 0;
    if (id != BMP280_CHIP_ID_VALUE)
        return 0;

    /* 3. 软复位，让芯片回到刚上电的干净状态。写失败也返回。 */
    if (BMP280_WriteReg(&bmp280_dev, BMP280_REG_RESET, BMP280_RESET_VALUE) == 0)
        return 0;
    HAL_Delay(10);                          /* 数据手册要求复位后至少等 2ms */

    /* 4. 读 24 字节校准系数（0x88 ~ 0x9F）。
     *    数据手册规定是"低字节在前"（小端），所以两个字节要倒过来拼：
     *    低字节 + 高字节<<8。 */
    if (BMP280_ReadRegs(&bmp280_dev, BMP280_REG_CALIB, calib_buf, 24) == 0)
        return 0;

    bmp280_dev.calib.dig_T1 = (uint16_t)(calib_buf[1]  << 8) | calib_buf[0];
    bmp280_dev.calib.dig_T2 = (int16_t) (calib_buf[3]  << 8) | calib_buf[2];
    bmp280_dev.calib.dig_T3 = (int16_t) (calib_buf[5]  << 8) | calib_buf[4];
    bmp280_dev.calib.dig_P1 = (uint16_t)(calib_buf[7]  << 8) | calib_buf[6];
    bmp280_dev.calib.dig_P2 = (int16_t) (calib_buf[9]  << 8) | calib_buf[8];
    bmp280_dev.calib.dig_P3 = (int16_t) (calib_buf[11] << 8) | calib_buf[10];
    bmp280_dev.calib.dig_P4 = (int16_t) (calib_buf[13] << 8) | calib_buf[12];
    bmp280_dev.calib.dig_P5 = (int16_t) (calib_buf[15] << 8) | calib_buf[14];
    bmp280_dev.calib.dig_P6 = (int16_t) (calib_buf[17] << 8) | calib_buf[16];
    bmp280_dev.calib.dig_P7 = (int16_t) (calib_buf[19] << 8) | calib_buf[18];
    bmp280_dev.calib.dig_P8 = (int16_t) (calib_buf[21] << 8) | calib_buf[20];
    bmp280_dev.calib.dig_P9 = (int16_t) (calib_buf[23] << 8) | calib_buf[22];

    /* 5. 配置工作参数：温度×1、气压×16、先睡眠（还没让芯片开始测） */
    if (BMP280_WriteReg(&bmp280_dev, BMP280_REG_CTRL_MEAS, BMP280_CTRL_SLEEP) == 0)
        return 0;
    /*    再配滤波系数×4 + 待机 0.5ms，让读出来的数值更稳不容易跳 */
    if (BMP280_WriteReg(&bmp280_dev, BMP280_REG_CONFIG,   BMP280_CONFIG_VAL) == 0)
        return 0;

    bmp280_online = 1;                      /* 全流程走通，标记在线 */
    return 1;
}

/**
 * @brief  触发一次测量（forced 模式），写完立刻返回
 * @retval 1 = 命令已发出；0 = 传感器不在线 或 写入失败
 * @note   芯片要花约 41ms 转换，这个等待放在 BMP280_Get() 里做（查状态位）
 */
uint8_t BMP280_Measure(void)
{
    if (bmp280_online == 0) return 0;       /* 没初始化过，直接失败 */

    /* 0x35 = 温度×1 + 气压×16 + forced 单次模式
     * forced 模式：测完这一次就自动休眠，省电，适合"每秒读一次"的场景 */
    return BMP280_WriteReg(&bmp280_dev, BMP280_REG_CTRL_MEAS, BMP280_CTRL_FORCED);
}

/**
 * @brief  等转换完成 + 读出 6 字节原始数据（阻塞，但带超时，不会死等）
 * @retval 1 = 数据已新鲜读回；0 = 传感器不在线 / 读失败 / 转换超时
 * @note   和旧版最大的区别：先查状态寄存器(0xF3)的 bit3，
 *         确认"正在测量"这个标志清零了才去读数据，
 *         这样拿到的必定是刚测完的新数据，而不是上一轮的旧数据。
 */
uint8_t BMP280_Get(void)
{
    uint8_t  status = 0;
    uint32_t tick;

    if (bmp280_online == 0) return 0;

    /* 1. 等转换完成：bit3(0x08) 为 1 表示"还在测量"，一直等它变 0。
     *    用 HAL_GetTick() 做超时判断，最多等 BMP280_I2C_TIMEOUT 毫秒，
     *    超时说明芯片可能没接好，返回 0 而不是死等。 */
    tick = HAL_GetTick();
    do
    {
        if (BMP280_ReadReg(&bmp280_dev, BMP280_REG_STATUS, &status) == 0)
            return 0;                       /* 读状态都失败，没戏了 */
        if (HAL_GetTick() - tick > BMP280_I2C_TIMEOUT)
            return 0;                       /* 超时：转换一直没完成 */
    } while (status & 0x08);

    /* 2. 数据新鲜了，从 0xF7 连读 6 字节（气压 3 字节 + 温度 3 字节） */
    return BMP280_ReadRegs(&bmp280_dev, BMP280_REG_PRESS_MSB, bmp280_rx_buffer, 6);
}

/**
 * @brief  把缓冲区里的原始数据换算成真实的气压和温度
 * @param  pressure    输出：气压，单位 hPa（百帕，标准大气压约 1013）
 * @param  temperature 输出：温度，单位 ℃
 * @retval 1 = 换算成功；0 = 传感器不在线 或 校准系数异常
 * @note   气压补偿必须放在温度补偿后面 —— 因为要用到温度算出来的 t_fine
 */
uint8_t BMP280_Analysis(float *pressure, float *temperature)
{
    int32_t temp_raw, press_raw;
    int32_t var1, var2, T;
    int64_t p_var1, p_var2, p;

    /* 防御：没初始化、或校准系数没读到（dig_T1 还是 0），都直接返回失败，
     * 绝不用全 0 的校准系数算出一堆垃圾值。 */
    if (bmp280_online == 0) return 0;
    if (bmp280_dev.calib.dig_T1 == 0) return 0;

    if (pressure == 0 && temperature == 0)  /* 两个指针都是空，没地方放结果 */
        return 0;

    /* 1. 拼出 20 位原始值 = 高8位<<12 | 低8位<<4 | 小数位>>4
     *    （XLSB 只有高 4 位有效，低 4 位是无效的小数位，丢掉） */
    press_raw = ((int32_t)bmp280_rx_buffer[0] << 12) |
                ((int32_t)bmp280_rx_buffer[1] << 4)  |
                (bmp280_rx_buffer[2] >> 4);

    temp_raw  = ((int32_t)bmp280_rx_buffer[3] << 12) |
                ((int32_t)bmp280_rx_buffer[4] << 4)  |
                (bmp280_rx_buffer[5] >> 4);

    /* 2. 温度补偿（数据手册 Section 3.11.3 的整数算法，照抄即可）
     *    算出来的 t_fine 是个中间量，第 3 步算气压还要用 */
    var1 = ((((temp_raw >> 3) - ((int32_t)bmp280_dev.calib.dig_T1 << 1))) *
            ((int32_t)bmp280_dev.calib.dig_T2)) >> 11;

    var2 = (((((temp_raw >> 4) - ((int32_t)bmp280_dev.calib.dig_T1)) *
              ((temp_raw >> 4) - ((int32_t)bmp280_dev.calib.dig_T1))) >> 12) *
            ((int32_t)bmp280_dev.calib.dig_T3)) >> 14;

    bmp280_dev.t_fine = var1 + var2;
    T = (bmp280_dev.t_fine * 5 + 128) >> 8;   /* 单位是 0.01℃ */

    /* 3. 气压补偿（必须放在温度后面，因为要用 t_fine）
     *    这里用 int64_t（64 位整数）：中间结果太大，32 位装不下会溢出算错。 */
    p_var1 = ((int64_t)bmp280_dev.t_fine) - 128000;
    p_var2 = p_var1 * p_var1 * (int64_t)bmp280_dev.calib.dig_P6;
    p_var2 = p_var2 + ((p_var1 * (int64_t)bmp280_dev.calib.dig_P5) << 17);
    p_var2 = p_var2 + (((int64_t)bmp280_dev.calib.dig_P4) << 35);
    p_var1 = ((p_var1 * p_var1 * (int64_t)bmp280_dev.calib.dig_P3) >> 8) +
             ((p_var1 * (int64_t)bmp280_dev.calib.dig_P2) << 12);
    p_var1 = (((((int64_t)1) << 47) + p_var1)) * ((int64_t)bmp280_dev.calib.dig_P1) >> 33;

    if (p_var1 == 0) {          /* 防止除以 0（校准系数异常时才会发生） */
        if (temperature) *temperature = (float)T / 100.0f;
        if (pressure)    *pressure = 0.0f;
        return 0;
    }

    p = 1048576 - press_raw;
    p = (((p << 31) - p_var2) * 3125) / p_var1;
    p_var1 = (((int64_t)bmp280_dev.calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    p_var2 = (((int64_t)bmp280_dev.calib.dig_P8) * p) >> 19;
    p = ((p + p_var1 + p_var2) >> 8) + (((int64_t)bmp280_dev.calib.dig_P7) << 4);

    /* 4. 输出。关键单位换算：数据手册规定这一步算出来的是 Q24.8 格式，
     *    也就是"真实帕斯卡数 × 256"（手册原话：24674867 代表 24674867/256 = 96386 Pa）
     *      除以 256   -> Pa（帕）
     *      再除以 100 -> hPa（百帕），平时说的"1013 百帕"就是它 */
    if (temperature) *temperature = (float)T / 100.0f;   /* 0.01℃ -> ℃ */
    if (pressure)    *pressure    = (float)p / 25600.0f; /* Q24.8 -> hPa */

    return 1;
}

/**
 * @brief  查询传感器是否初始化成功过
 * @retval 1 = 在线（Init 成功过）；0 = 离线
 */
uint8_t BMP280_IsOnline(void)
{
    return bmp280_online;
}

/**
 * @brief  读取芯片 ID（调试用）
 * @retval 正常返回 0x58；返回 0 说明没读到（地址错 / 线没接好）
 */
uint8_t BMP280_Read_ID(void)
{
    uint8_t id = 0;
    if (BMP280_ReadReg(&bmp280_dev, BMP280_REG_CHIP_ID, &id) == 0)
        return 0;
    return id;
}
