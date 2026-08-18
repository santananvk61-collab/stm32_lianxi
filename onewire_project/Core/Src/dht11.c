/*
 * dht11.c —— DHT11 温湿度读取实现
 *
 * 关键点 1：PA0 被 CubeMX 配成了 "开漏输出 + 上拉"。
 *   写 0 = 把线拉低；写 1 = 松手（靠外部/内部上拉电阻拉成高）。
 *   读 = 读引脚真实电平。因为双方都只能拉低，所以不会冲突（见之前讲的"线与"）。
 *
 * 关键点 2：DHT11 时序是微秒级的，需要精确的微秒延时。
 *   这里用内核自带的 DWT 循环计数器：每过一个 CPU 时钟周期 CYCCNT+1。
 *   72MHz 下，1 微秒 = 72 个周期。比 HAL_Delay(1ms) 精确得多。
 */
#include "dht11.h"

/* DHT11 数据引脚：PA0 */
#define DHT11_PIN   GPIO_PIN_0
#define DHT11_PORT  GPIOA

/* ---------------- 微秒延时（基于 DWT 循环计数器） ---------------- */
static void DWT_DelayUs(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = us * (SystemCoreClock / 1000000);  /* SystemCoreClock = 72000000 */
    while ((DWT->CYCCNT - start) < cycles);
}

/* 开启 DWT 计数器（上电后调一次） */
void DHT11_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;  /* 打开内核跟踪 */
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;             /* 开启 CYCCNT 计数 */
    DWT->CYCCNT = 0;
}

/* 在 timeout_us 时间内，等引脚变成 level；等到返回 0，超时返回 1 */
static uint8_t DHT11_WaitFor(uint8_t level, uint32_t timeout_us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t limit = timeout_us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < limit)
    {
        if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == level)
            return 0;   /* 等到了 */
    }
    return 1;           /* 超时 */
}

/* ---------------- 读取一次温湿度 ---------------- */
DHT11_Status DHT11_Read(int8_t *temperature, uint8_t *humidity)
{
    uint8_t data[5] = {0};

    /* 1) MCU 发开始信号：拉低 >= 18ms，然后释放（松手，线被上拉成高） */
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    DWT_DelayUs(19000);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);  /* 开漏：写1=松手 */
    DWT_DelayUs(30);   /* 等 20~40us，给传感器反应时间 */

    /* 2) 等传感器应答：它先拉低 80us，再拉高 80us，然后拉低（数据开始） */
    if (DHT11_WaitFor(GPIO_PIN_RESET, 100)) return DHT11_ERR_TIMEOUT;
    if (DHT11_WaitFor(GPIO_PIN_SET,  100)) return DHT11_ERR_TIMEOUT;
    if (DHT11_WaitFor(GPIO_PIN_RESET, 100)) return DHT11_ERR_TIMEOUT;

    /* 3) 读 40 位：每一位 = 先 50us 低，再一段高（26~28us=0，70us=1） */
    for (uint8_t i = 0; i < 40; i++)
    {
        if (DHT11_WaitFor(GPIO_PIN_SET, 100)) return DHT11_ERR_TIMEOUT; /* 等这一位变高 */
        DWT_DelayUs(40);                                                /* 在 40us 处采样判断 */
        data[i / 8] <<= 1;
        if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET)
            data[i / 8] |= 1;                                           /* 还是高 => 这一位是 1 */
        if (DHT11_WaitFor(GPIO_PIN_RESET, 100)) return DHT11_ERR_TIMEOUT; /* 等这一位结束（变低） */
    }

    /* 4) 校验和：前 4 字节之和的低 8 位 应等于第 5 字节 */
    if (data[4] != (uint8_t)(data[0] + data[1] + data[2] + data[3]))
        return DHT11_ERR_CHECKSUM;

    *humidity    = data[0];   /* DHT11 的整数湿度（%） */
    *temperature = data[2];   /* DHT11 的整数温度（℃） */
    return DHT11_OK;
}
