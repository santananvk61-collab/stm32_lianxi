/**
  * @file    attitude.c
  * @brief   互补滤波姿态解算
  * @note   ICM-42670 安装方向未定，以下宏用于 S1 标定轴序与极性：
  *         手扶车体缓前倾，pitch 应增大；不满足时一次只改一项。
  */

#include <math.h>
#include "attitude.h"
#include "main.h"

/* ===== S1 轴映射标定 ===== */
#define ATT_IDX_UP           2       /* 竖直轴（平放时读到 ±1g） */
#define ATT_IDX_PITCH        0       /* 俯仰加速度轴 */
#define ATT_IDX_GYRO_PITCH   1       /* 俯仰陀螺轴 */
#define ATT_IDX_GYRO_YAW     2       /* 偏航陀螺轴 */
#define ATT_ACC_PITCH_SIGN  (-1.0f)
#define ATT_PITCH_GYRO_SIGN (-1.0f)
#define ATT_YAW_GYRO_SIGN   (+1.0f)
/* ========================= */

#define ATT_RAD2DEG          57.29578f
#define ATT_CAL_SAMPLES      400u
#define ATT_CAL_PERIOD_MS    5u
#define ATT_CAL_MAX_FAIL     40u

Attitude_t g_att = { .state = ATT_CALIBRATING };

static float s_angle;        /* 融合角度，前倾为正 */
static float s_acc_w = ATT_ACC_W_STATIC;

void Attitude_Init(void)
{
    uint8_t i;

    for (i = 0u; i < 3u; i++) {
        g_att.gyro_offset[i] = 0.0f;
    }
    s_angle          = 0.0f;
    s_acc_w          = ATT_ACC_W_STATIC;
    g_att.pitch      = 0.0f;
    g_att.pitch_rate = 0.0f;
    g_att.yaw_rate   = 0.0f;
    g_att.state      = ATT_CALIBRATING;
}

void Attitude_SetAccelWeight(float w)
{
    s_acc_w = w;
}

/* 加速度计瞬时倾角 */
static float acc_angle(const ICM_Data_t *d)
{
    return atan2f(ATT_ACC_PITCH_SIGN * d->acc_g[ATT_IDX_PITCH], d->acc_g[ATT_IDX_UP]) * ATT_RAD2DEG;
}

/* 静止采样 400 点求三轴零偏；以加速度角作为融合初值，避免上电后角度从 0 爬升 */
uint8_t Attitude_Calibrate(void)
{
    ICM_Data_t d;
    float   sum[3] = { 0.0f, 0.0f, 0.0f };
    uint16_t got  = 0u;
    uint16_t miss = 0u;

    g_att.state = ATT_CALIBRATING;

    while (got < ATT_CAL_SAMPLES) {
        if (ICM_ReadData(&d) != 0u) {
            if (++miss >= ATT_CAL_MAX_FAIL) {
                return 1u;
            }
            continue;
        }
        miss = 0u;
        sum[0] += d.gyro_dps[0];
        sum[1] += d.gyro_dps[1];
        sum[2] += d.gyro_dps[2];
        got++;
        HAL_Delay(ATT_CAL_PERIOD_MS);
    }

    g_att.gyro_offset[0] = sum[0] / (float)ATT_CAL_SAMPLES;
    g_att.gyro_offset[1] = sum[1] / (float)ATT_CAL_SAMPLES;
    g_att.gyro_offset[2] = sum[2] / (float)ATT_CAL_SAMPLES;

    s_angle          = acc_angle(&d);
    g_att.pitch      = s_angle;
    g_att.pitch_rate = 0.0f;
    g_att.yaw_rate   = 0.0f;
    g_att.state      = ATT_READY;

    return 0u;
}

/* 互补滤波：angle = (1-w)*(angle + gyro*dt) + w*angle_acc */
uint8_t Attitude_Update(const ICM_Data_t *d, float dt)
{
    float gy;
    float gz;
    float a_acc;

    if ((d == 0) || (g_att.state != ATT_READY)) {
        return 1u;
    }

    gy = ATT_PITCH_GYRO_SIGN * (d->gyro_dps[ATT_IDX_GYRO_PITCH] - g_att.gyro_offset[ATT_IDX_GYRO_PITCH]);
    gz = ATT_YAW_GYRO_SIGN   * (d->gyro_dps[ATT_IDX_GYRO_YAW]   - g_att.gyro_offset[ATT_IDX_GYRO_YAW]);

    a_acc = acc_angle(d);
    s_angle = (1.0f - s_acc_w) * (s_angle + gy * dt) + s_acc_w * a_acc;

    if (s_angle > 180.0f) {
        s_angle -= 360.0f;
    } else if (s_angle < -180.0f) {
        s_angle += 360.0f;
    }

    g_att.pitch      = s_angle;
    g_att.pitch_rate = gy;
    g_att.yaw_rate   = gz;

    return 0u;
}
