/**
  * @file    balance.c
  * @brief   直立环 PD + 速度环 PI + 转向环 + 安全状态机
  * @note   结构移植自 phc（实车验证），仅传感器/驱动/时基不同。
  *         参数按原车 PWM 量程 7199 整定，末端用 BAL_OUT_SCALE 等比映射到本车 4199，
  *         占空比等价，因此参数无需换算。
  */

#include <math.h>
#include "balance.h"
#include "attitude.h"
#include "bsp_at8236.h"
#include "bsp_encoder.h"
#include "bsp_spi_icm42670.h"

/* 控制周期 5ms；速度环 40ms；参数按原车 7199 量程整定，末端按占空比等价映射 */
#define BAL_CTRL_DT         0.005f
#define BAL_SPD_DIV         8u
#define BAL_SPD_A           0.7f
#define BAL_SPD_DECAY       0.98f
#define BAL_REF_PWM_MAX     7199.0f
#define BAL_OUT_SCALE       ((float)MOTOR_PWM_MAX / BAL_REF_PWM_MAX)
#define BAL_ARM_HOLD        200u    /* 1s 稳定窗口 */
#define BAL_ARM_GYRO_MAX    8.0f
#define BAL_RECOVER_HOLD    40u     /* 0.2s */
#define BAL_DISARM_ANGLE    25.0f
#define BAL_FORCE_ARM_ANGLE 8.0f
#define BAL_IMU_FAIL_MAX    20u     /* 100ms */
#define BAL_SPEED_TARGET_MAX 200.0f
#define BAL_TURN_MANUAL_MAX 1200.0f

/* 初值移植自 phc 实车验证参数，可经串口在线改写；字段含义见 balance.h */
Bal_Param_t g_param = {
    .angle_kp          = 400.0f,
    .angle_kd          = 15.0f,
    .speed_kp          = 10.0f,
    .speed_ki          = 0.8f,
    .speed_angle_scale = 0.02f,
    .speed_i_max       = 500.0f,
    .speed_out_max     = 500.0f,
    .turn_kp           = 0.0f,
    .turn_max          = 1200.0f,
    .mech_angle        = 0.0f,   /* 本车需实测（原车 4.5°），串口 ANG= 设置 */
    .arm_angle         = 2.5f,
    .fall_angle        = 35.0f,
    .recover_angle     = 8.0f,
    .dz_in             = 300.0f,
    .dz_out            = 100.0f,
    .lr_kp             = 10.0f,
    .lr_ki             = 0.6f,
    .lr_i_max          = 500.0f,
    .lr_out_max        = 1000.0f,
};

/* ---------------- 运行时状态 ---------------- */
static Bal_State_t s_state = BAL_CAL;

static float   s_spd_filter;    /* 速度低通 */
static float   s_spd_integral;  /* 速度积分（衰减式） */
static float   s_spd_out;       /* 速度环输出（虚拟 PWM，原车单位） */
static int32_t s_enc_acc_l;     /* 40ms 内左轮计数累计 */
static int32_t s_enc_acc_r;
static uint8_t s_spd_div;

static float   s_lr_integral;   /* 左右均衡积分 */
static float   s_lr_diff;       /* 左右均衡差速输出 */

static float   s_target_angle;  /* 直立环目标角 */
static float   s_target_speed;  /* 遥控目标速度，计数/40ms */
static float   s_turn_manual;   /* 遥控转向差速 */
static float   s_vbat;

static uint16_t s_arm_cnt;
static uint8_t  s_arm_ready;
static uint8_t  s_auto_arm = 1u;  /* 1=允许车身立正 1s 后自动启动；RUN=0 后锁存为 0 */
static uint16_t s_recover_cnt;
static uint16_t s_imu_fail;

static Bal_Debug_t s_db;

/* 清空速度环/均衡环状态（启动、倒地、重校准各调用一次） */
static void control_reset(void)
{
    s_spd_filter   = 0.0f;
    s_spd_integral = 0.0f;
    s_spd_out      = 0.0f;
    s_enc_acc_l    = 0;
    s_enc_acc_r    = 0;
    s_spd_div      = 0u;
    s_lr_integral  = 0.0f;
    s_lr_diff      = 0.0f;
    s_target_angle = g_param.mech_angle;
    s_arm_cnt      = 0u;
    s_arm_ready    = 0u;
}

/* 死区补偿：a + DZ_OUT*a/(a+DZ_IN)，a=|pwm|。
 * 小信号放大以克服减速箱静摩擦，大信号补偿自动衰减；
 * 不能改为固定叠加，否则平衡点会变成 0/DZ_OUT 开关振荡。 */
static float dz_compensate(float pwm)
{
    float a = fabsf(pwm);

    a = a + g_param.dz_out * (a / (a + g_param.dz_in));
    return (pwm < 0.0f) ? -a : a;
}

/* ---------- 进入闭环 ---------- */
static void arm_now(void)
{
    control_reset();
    Encoder_Reset();
    s_state     = BAL_RUN;
    s_arm_ready = 1u;
}

/* ---------- 进入倒地保护 ---------- */
static void enter_fall(void)
{
    s_state = BAL_FALL;
    control_reset();
    s_recover_cnt = 0u;
    Motor_Brake();
}

/* Balance_Init：进入待机（姿态须已校准），电机刹车 */
void Balance_Init(void)
{
    /* 姿态未校准成功就不进待机，停在 CAL：只刹车、不读 SPI，保证安全 */
    s_state        = (g_att.state == ATT_READY) ? BAL_READY : BAL_CAL;
    s_auto_arm     = 1u;
    s_target_speed = 0.0f;
    s_turn_manual  = 0.0f;
    s_vbat         = 0.0f;
    s_imu_fail     = 0u;
    s_recover_cnt  = 0u;
    control_reset();
    Encoder_Reset();
    Attitude_SetAccelWeight(ATT_ACC_W_STATIC);
    Motor_Brake();
}

/* Balance_Step：5ms 控制链（TIM5 更新中断内执行） */
void Balance_Step(void)
{
    ICM_Data_t imu;
    float angle;
    float gy;
    float gz;
    float bal;
    float turn;
    float pwm_l;
    float pwm_r;

    /* 0) 校准态：不读 SPI，避免与主循环的阻塞校准重入 */
    if (s_state == BAL_CAL) {
        Motor_Brake();
        return;
    }

    /* 1) 读取失败即刹车；连续 100ms 失败判传感器失效 -> 倒地保护 */
    if (ICM_ReadData(&imu) != 0u) {
        Motor_Brake();
        if (++s_imu_fail >= BAL_IMU_FAIL_MAX) {
            enter_fall();
        }
        return;
    }
    s_imu_fail = 0u;

    if (Attitude_Update(&imu, BAL_CTRL_DT) != 0u) {
        Motor_Brake();
        return;
    }

    angle = g_att.pitch;
    gy    = g_att.pitch_rate;
    gz    = g_att.yaw_rate;

    /* 2) 编码器 5ms 读清，累加到 40ms 缓冲 */
    s_enc_acc_l += Encoder_ReadLeft();
    s_enc_acc_r += Encoder_ReadRight();

    s_db.angle = angle;
    s_db.state = s_state;

    /* 3) 倒地 / 恢复 */
    if (s_state == BAL_FALL) {
        if (fabsf(angle) < g_param.recover_angle) {
            if (++s_recover_cnt >= BAL_RECOVER_HOLD) {
                s_recover_cnt = 0u;
                control_reset();
                s_state    = BAL_READY;
                s_auto_arm = 1u;   /* 倒地抬正后恢复自动启动（与原工程一致） */
            }
        } else {
            s_recover_cnt = 0u;
        }
        Motor_Brake();
        return;
    }
    if (fabsf(angle) > g_param.fall_angle) {
        enter_fall();
        return;
    }

    /* 4) 待机：刹车；立正静止 1s 自动启动 */
    if (s_state == BAL_READY) {
        Motor_Brake();
        s_target_angle = g_param.mech_angle;
        s_db.target_angle = s_target_angle;
        s_db.speed    = 0.0f;
        s_db.pwm[0]   = 0;
        s_db.pwm[1]   = 0;

        s_arm_ready = 0u;
        if (s_auto_arm
            && (fabsf(angle - g_param.mech_angle) < g_param.arm_angle)
            && (fabsf(gy) < BAL_ARM_GYRO_MAX)) {
            s_arm_ready = 1u;
            if (s_arm_cnt < BAL_ARM_HOLD) {
                s_arm_cnt++;
            }
            if (s_arm_cnt >= BAL_ARM_HOLD) {
                arm_now();      /* 稳 1s，自动进入闭环 */
            }
        } else {
            s_arm_cnt = 0u;
        }
        s_db.arm_ready = s_arm_ready;
        s_db.armed     = 0u;
        s_db.speed_out = 0.0f;
        return;
    }

    /* 5) 扳到 25° 外 = 手动停车 */
    if (fabsf(angle - g_param.mech_angle) > BAL_DISARM_ANGLE) {
        control_reset();
        s_state = BAL_READY;
        Motor_Brake();
        return;
    }

    /* 6) 速度环 PI（40ms）：e = v - 目标速度，target=0 时与原车等价 */
    if (++s_spd_div >= BAL_SPD_DIV) {
        float v = (float)(s_enc_acc_l + s_enc_acc_r) * 0.5f;
        float d = (float)(s_enc_acc_l - s_enc_acc_r);   /* 左快为正 */

        s_spd_div   = 0u;
        s_enc_acc_l = 0;
        s_enc_acc_r = 0;

        s_spd_filter   = BAL_SPD_A * s_spd_filter + (1.0f - BAL_SPD_A) * v;
        s_spd_integral = BAL_SPD_DECAY * s_spd_integral + (v - s_target_speed);
        if (s_spd_integral >  g_param.speed_i_max) s_spd_integral =  g_param.speed_i_max;
        if (s_spd_integral < -g_param.speed_i_max) s_spd_integral = -g_param.speed_i_max;

        s_spd_out = g_param.speed_kp * (s_spd_filter - s_target_speed)
                  + g_param.speed_ki * s_spd_integral;
        if (s_spd_out >  g_param.speed_out_max) s_spd_out =  g_param.speed_out_max;
        if (s_spd_out < -g_param.speed_out_max) s_spd_out = -g_param.speed_out_max;

        /* 7) 左右均衡：左快(d>0) -> 左减右加 */
        s_lr_integral = BAL_SPD_DECAY * s_lr_integral + d;
        if (s_lr_integral >  g_param.lr_i_max) s_lr_integral =  g_param.lr_i_max;
        if (s_lr_integral < -g_param.lr_i_max) s_lr_integral = -g_param.lr_i_max;

        s_lr_diff = g_param.lr_kp * d + g_param.lr_ki * s_lr_integral;
        if (s_lr_diff >  g_param.lr_out_max) s_lr_diff =  g_param.lr_out_max;
        if (s_lr_diff < -g_param.lr_out_max) s_lr_diff = -g_param.lr_out_max;

        s_db.speed     = s_spd_filter;
        s_db.speed_out = s_spd_out;
    }

    /* 8) 直立环 PD：target = mech - SCALE*spd_out（负号，实测）；D 项直接用陀螺 */
    s_target_angle = g_param.mech_angle - g_param.speed_angle_scale * s_spd_out;
    bal = g_param.angle_kp * (angle - s_target_angle) + g_param.angle_kd * gy;

    /* 9) 转向环 P（默认关，恢复时从 1.0 起步） */
    turn = g_param.turn_kp * gz;
    if (turn >  g_param.turn_max) turn =  g_param.turn_max;
    if (turn < -g_param.turn_max) turn = -g_param.turn_max;

    /* 10) 合成：左 = 直立 + 转向 + 遥控 - 均衡，右取反 */
    pwm_l = bal + turn + s_turn_manual - s_lr_diff;
    pwm_r = bal - turn - s_turn_manual + s_lr_diff;

    /* 11) 死区补偿 -> 限幅(原车单位) -> 等比映射到本车 PWM */
    pwm_l = dz_compensate(pwm_l) * BAL_OUT_SCALE;
    pwm_r = dz_compensate(pwm_r) * BAL_OUT_SCALE;

    if (pwm_l >  (float)MOTOR_PWM_MAX) pwm_l =  (float)MOTOR_PWM_MAX;
    if (pwm_l < -(float)MOTOR_PWM_MAX) pwm_l = -(float)MOTOR_PWM_MAX;
    if (pwm_r >  (float)MOTOR_PWM_MAX) pwm_r =  (float)MOTOR_PWM_MAX;
    if (pwm_r < -(float)MOTOR_PWM_MAX) pwm_r = -(float)MOTOR_PWM_MAX;

    Motor_Set((int16_t)pwm_l, (int16_t)pwm_r);

    /* 12) 运动中压低加速度权重，抑制假倾角正反馈；不设 0 以纠陀螺漂移 */
    if ((fabsf(s_spd_filter) < 8.0f) && (fabsf(gy) < 10.0f)) {
        Attitude_SetAccelWeight(ATT_ACC_W_STATIC);
    } else {
        Attitude_SetAccelWeight(ATT_ACC_W_MOVING);
    }

    s_db.target_angle = s_target_angle;
    s_db.pwm[0]       = (int16_t)pwm_l;
    s_db.pwm[1]       = (int16_t)pwm_r;
    s_db.armed        = 1u;
    s_db.arm_ready    = 1u;
}

/* RUN=1 手工启动；1=已启动，0=姿态超出允许范围 */
uint8_t Balance_RequestArm(void)
{
    if (s_state == BAL_RUN) {
        return 1u;
    }
    if (s_state != BAL_READY) {
        return 0u;
    }
    if (fabsf(g_att.pitch - g_param.mech_angle) > BAL_FORCE_ARM_ANGLE) {
        return 0u;
    }
    s_auto_arm = 1u;
    arm_now();
    return 1u;
}

/* 退出闭环并锁存自动启动；需 RUN=1 重新出力 */
void Balance_Stop(void)
{
    if (s_state == BAL_CAL) {
        return;
    }
    s_auto_arm = 0u;
    if (s_state == BAL_FALL) {
        Motor_Brake();   /* 倒地保护优先，保持 FALL，等抬正后自动恢复 */
        return;
    }
    control_reset();
    s_state = BAL_READY;
    Motor_Brake();
}

/* 重新校准陀螺零偏（阻塞约 2s，要求静止）；CAL 态仅刹车、不读 SPI */
void Balance_Recalibrate(void)
{
    s_state = BAL_CAL;
    control_reset();
    Motor_Brake();
    Encoder_Reset();

    if (Attitude_Calibrate() == 0u) {
        control_reset();
        s_state    = BAL_READY;
        s_auto_arm = 1u;
    } else {
        enter_fall();
    }
    Motor_Brake();
}

Bal_State_t Balance_GetState(void)
{
    return s_state;
}

uint8_t Balance_IsArmReady(void)
{
    return s_arm_ready;
}

/* Balance_SetTargetSpeed：遥控前进目标（编码器计数/40ms，正=前进）。 */
void Balance_SetTargetSpeed(float cnt_per_40ms)
{
    if (cnt_per_40ms >  BAL_SPEED_TARGET_MAX) cnt_per_40ms =  BAL_SPEED_TARGET_MAX;
    if (cnt_per_40ms < -BAL_SPEED_TARGET_MAX) cnt_per_40ms = -BAL_SPEED_TARGET_MAX;
    s_target_speed = cnt_per_40ms;
}

/* Balance_SetTurn：遥控转向差速（正=右转，左轮加、右轮减）。 */
void Balance_SetTurn(float manual_diff)
{
    if (manual_diff >  BAL_TURN_MANUAL_MAX) manual_diff =  BAL_TURN_MANUAL_MAX;
    if (manual_diff < -BAL_TURN_MANUAL_MAX) manual_diff = -BAL_TURN_MANUAL_MAX;
    s_turn_manual = manual_diff;
}

void Balance_NotifyVbat(float v)
{
    s_vbat = v;
}

/* Balance_GetDebug：拷贝调试快照给协议层上传。 */
void Balance_GetDebug(Bal_Debug_t *out)
{
    if (out == 0) {
        return;
    }
    *out = s_db;
    out->target_speed = s_target_speed;
    out->turn_manual  = s_turn_manual;
    out->vbat         = s_vbat;
    out->state        = s_state;
    out->arm_ready    = s_arm_ready;
    out->armed        = (s_state == BAL_RUN) ? 1u : 0u;
}
