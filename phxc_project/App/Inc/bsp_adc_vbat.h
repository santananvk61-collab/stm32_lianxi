/**
  * @file    bsp_adc_vbat.h
  * @brief   电池电压采集（ADC1_IN9 = PB1）接口
  */

#ifndef __BSP_ADC_VBAT_H
#define __BSP_ADC_VBAT_H

#include <stdint.h>

void  Vbat_Init(void);
float Vbat_Read(void);   /* 返回电池电压，单位 V，量程 14.19V */

#endif /* __BSP_ADC_VBAT_H */
