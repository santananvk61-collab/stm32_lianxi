/**
  * @file    bsp_adc_vbat.c
  * @brief   电池电压采集（ADC1_IN9 = PB1）
  * @note   33k/10k 分压（1:4.3）；单次软件触发，连采 4 次平均。
  */

#include "bsp_adc_vbat.h"
#include "adc.h"

#define VBAT_ADC_VREF       3.3f
#define VBAT_ADC_FULLSCALE  4095.0f   /* 12bit */
#define VBAT_DIV_RATIO      4.3f      /* (33k+10k)/10k */
#define VBAT_AVG_TIMES      4u
#define VBAT_POLL_MS        5u

/* F4 ADC 出厂校准，无需初始化 */
void Vbat_Init(void)
{
}

/* 连采 4 次平均并按分压比换算（V） */
float Vbat_Read(void)
{
    uint32_t sum = 0u;
    uint32_t i;
    float adc_avg;

    for (i = 0u; i < VBAT_AVG_TIMES; i++) {
        if (HAL_ADC_Start(&hadc1) == HAL_OK) {
            if (HAL_ADC_PollForConversion(&hadc1, VBAT_POLL_MS) == HAL_OK) {
                sum += HAL_ADC_GetValue(&hadc1);
            }
            (void)HAL_ADC_Stop(&hadc1);
        }
    }

    adc_avg = (float)sum / (float)VBAT_AVG_TIMES;
    return adc_avg * (VBAT_ADC_VREF / VBAT_ADC_FULLSCALE) * VBAT_DIV_RATIO;
}
