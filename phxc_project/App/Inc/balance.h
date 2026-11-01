/**
  * @file    balance.h
  * @brief   直立环 PD + 速度环 PI + 转向环 + 安全状态机
  * @note   5ms 直立环在 TIM5 更新中断内执行；速度环/均衡环 40ms。
  *         速度环输出不直接给 PWM，而是平移目标角度（再被 KP 放大数百倍）。
  */

#ifndef __BALANCE_H
#define __BALANCE_H

#include <stdint.h>

typedef enum { BAL_CAL = 0, BAL_READY, BAL_RUN, BAL_FALL } Bal_State_t;

/* ==================== 调参表（沿用 phc 实车验证值） ====================
 * 顺序：直立环 -> 速度环 -> 转向环，一次只改一个参数。
 *
 * 硬约束（均为实测结论）：
 *  1) speed_angle_scale >= 0.02：直立环静差约 370 PWM/度，目标角需多移约 4°
 *     才能兑现；过小则速度环被静差吃光，表现为“能站但一直加速”。
 *  2) 速度环符号取负：target = mech_angle - speed_angle_scale * spd_out。
 *     取正会形成后退加速的正反馈，约 1s 倒地。
 *  3) 死区补偿必须连续平滑：a + DZ_OUT*a/(a+DZ_IN)；固定叠加会在平衡点
 *     变成 0/DZ_OUT 开关振荡。
 *  4) KP=1000/KD=30 实测出现 19Hz 极限环，禁用；KD 不要单独加大。
 * ==================================================================== */

typedef struct {
    /* 直立环 */
    float angle_kp, angle_kd;
    /* 速度环 */
    float speed_kp, speed_ki, speed_angle_scale;
    float speed_i_max, speed_out_max;
    /* 转向环 */
    float turn_kp, turn_max;
    /* 姿态/安全 */
    float mech_angle;      /* 机械零点（悬停角）°，原车实测 4.5，本车需实测 */
    float arm_angle;       /* 启动窗口 ±°，原车实测 2.5 */
    float fall_angle;      /* 倒地阈值 °，原车实测 35 */
    float recover_angle;   /* 恢复阈值 °，原车实测 8，保持 0.2s */
    /* 死区补偿 */
    float dz_in, dz_out;   /* 原车实测 300 / 100 */
    /* 左右轮速均衡 */
    float lr_kp, lr_ki, lr_i_max, lr_out_max;  /* 原车实测 10 / 0.6 / 500 / 1000 */
} Bal_Param_t;

/* 调试快照（串口 50ms 上传 + INFO 查询） */
typedef struct {
    float       angle;         /* 融合角度 °（前倾为正） */
    float       target_angle;  /* 直立环目标角 ° */
    float       speed;         /* 实测速度：40ms 内平均编码器计数 */
    float       speed_out;     /* 速度环输出（虚拟 PWM，原车单位） */
    float       target_speed;  /* 遥控目标速度（计数/40ms） */
    float       turn_manual;   /* 遥控转向差速 */
    float       vbat;          /* 电池电压 V */
    int16_t     pwm[2];        /* 实际下发 PWM（本车单位） */
    Bal_State_t state;
    uint8_t     arm_ready;     /* 1=启动窗口内且姿态稳定 */
    uint8_t     armed;         /* 1=已出力 */
} Bal_Debug_t;

extern Bal_Param_t g_param;

void        Balance_Init(void);          /* 初始化（须在姿态校准完成后调用） */
void        Balance_Step(void);          /* 5ms 定周期控制链（TIM5 中断内调用） */
uint8_t     Balance_RequestArm(void);    /* 手工启动：1=已启动，0=姿态不在允许范围 */
void        Balance_Stop(void);          /* 退出闭环并锁存，刹车 */
void        Balance_Recalibrate(void);   /* 重新陀螺零偏校准（阻塞约 2s） */
Bal_State_t Balance_GetState(void);
uint8_t     Balance_IsArmReady(void);

void        Balance_SetTargetSpeed(float cnt_per_40ms);
void        Balance_SetTurn(float manual_diff);
void        Balance_NotifyVbat(float v);
void        Balance_GetDebug(Bal_Debug_t *out);

#endif /* __BALANCE_H */
