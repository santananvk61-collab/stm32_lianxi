/**
  * @file    attitude.h
  * @brief   陀螺零偏校准 + 互补滤波姿态解算
  * @note   融合以陀螺积分为主、加速度计慢速纠偏；运动中降低加速度权重，
  *         抑制加速引起的假倾角正反馈。输出约定：前倾为正。
  */

#ifndef __ATTITUDE_H
#define __ATTITUDE_H

#include <stdint.h>
#include "bsp_spi_icm42670.h"

typedef enum { ATT_CALIBRATING = 0, ATT_READY } Att_State_t;

typedef struct {
    float       pitch;          /* 俯仰角 °，前倾为正 */
    float       pitch_rate;     /* 俯仰角速度 °/s，前倾为正（已去零偏） */
    float       yaw_rate;       /* 偏航角速度 °/s（已去零偏） */
    float       gyro_offset[3]; /* 三轴陀螺零偏 °/s（SPI 原始轴序） */
    Att_State_t state;
} Attitude_t;

extern Attitude_t g_att;

/* 静止 / 运动中加速度计权重（实车验证值） */
#define ATT_ACC_W_STATIC    0.03f
#define ATT_ACC_W_MOVING    0.005f

void    Attitude_Init(void);
uint8_t Attitude_Calibrate(void);                       /* 阻塞约 2s，要求静止；0=成功 */
uint8_t Attitude_Update(const ICM_Data_t *d, float dt); /* 5ms 调用；0=数据可用 */
void    Attitude_SetAccelWeight(float w);

#endif /* __ATTITUDE_H */
