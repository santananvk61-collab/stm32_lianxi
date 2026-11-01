/**
  * @file    protocol.c
  * @brief   串口行指令解析、状态提示与 50ms CSV 波形
  * @note   上行：angle,target_angle,speed,pwm_l,pwm_r
  *         下行：KP= KD= VKP= VKI= VSA= KZ= ANG= DZIN= DZOUT= SP= TR=
  *               RUN=1/0  CAL  INFO  HELP
  *         浮点格式化自行实现，不依赖 newlib-nano 的 %f。
  */

#include <string.h>
#include "protocol.h"
#include "balance.h"
#include "bsp_uart.h"
#include "main.h"

#define PROTO_LINE_MAX    64u
#define PROTO_TX_PERIOD   50u   /* 波形上传周期 ms */

static char     s_line[PROTO_LINE_MAX];
static uint8_t  s_len;
static uint8_t  s_ready;
static uint32_t s_last_tx_tick;

/* ---- 轻量浮点解析/格式化 ---- */

/* 解析 "=±12.34" 形式的参数 */
static float parse_float(const char *s)
{
    float int_part = 0.0f;
    float frac     = 0.0f;
    float div      = 10.0f;
    float sign     = 1.0f;

    while ((*s == ' ') || (*s == '=')) {
        s++;
    }
    if (*s == '-') {
        sign = -1.0f;
        s++;
    } else if (*s == '+') {
        s++;
    }
    while ((*s >= '0') && (*s <= '9')) {
        int_part = int_part * 10.0f + (float)(*s - '0');
        s++;
    }
    if (*s == '.') {
        s++;
        while ((*s >= '0') && (*s <= '9')) {
            frac += (float)(*s - '0') / div;
            div *= 10.0f;
            s++;
        }
    }
    return sign * (int_part + frac);
}

/* 定点小数输出，dec=0~4 */
static void ftoa(float v, char *out, uint8_t dec)
{
    char    tmp[20];
    char    dig[12];
    uint8_t n = 0u;
    uint8_t k;
    uint8_t i;
    long    scale = 1L;
    long    ip;
    long    fp;

    if (v < 0.0f) {
        tmp[n++] = '-';
        v = -v;
    }
    for (i = 0u; i < dec; i++) {
        scale *= 10L;
    }
    v += 0.5f / (float)scale;          /* 末位四舍五入 */
    ip = (long)v;
    fp = (long)((v - (float)ip) * (float)scale);
    if (fp >= scale) {                 /* 舍入进位后小数满位 */
        ip += 1L;
        fp -= scale;
    }

    /* 整数部分 */
    k = 0u;
    do {
        dig[k++] = (char)('0' + (ip % 10L));
        ip /= 10L;
    } while (ip > 0L);
    while (k > 0u) {
        tmp[n++] = dig[--k];
    }

    /* 小数部分 */
    if (dec > 0u) {
        tmp[n++] = '.';
        k = 0u;
        for (i = 0u; i < dec; i++) {
            dig[k++] = (char)('0' + (fp % 10L));
            fp /= 10L;
        }
        while (k > 0u) {
            tmp[n++] = dig[--k];
        }
    }
    tmp[n] = '\0';
    (void)strcpy(out, tmp);
}

/* ---- 应答 ---- */

static void send_help(void)
{
    Uart_SendString("CMD: KP= KD= VKP= VKI= VSA= KZ= ANG= DZIN= DZOUT= SP= TR= RUN=1/0 CAL INFO HELP\n");
}

static void send_info(void)
{
    Bal_Debug_t db;
    char b[24];
    static const char *st_name[4] = { "CAL", "RDY", "RUN", "FAL" };  /* 状态机顺序 */

    Balance_GetDebug(&db);

    Uart_SendString("ST=");
    Uart_SendString(st_name[(uint8_t)db.state & 0x03u]);
    Uart_SendString(" ");
    Uart_SendString("ARM=");
    ftoa((float)db.armed, b, 0);
    Uart_SendString(b);
    Uart_SendString(" ANG=");
    ftoa(db.angle, b, 2);
    Uart_SendString(b);
    Uart_SendString(" TGT=");
    ftoa(db.target_angle, b, 2);
    Uart_SendString(b);
    Uart_SendString(" VBAT=");
    ftoa(db.vbat, b, 2);
    Uart_SendString(b);
    Uart_SendString("\r\n");

    Uart_SendString("KP=");
    ftoa(g_param.angle_kp, b, 2);
    Uart_SendString(b);
    Uart_SendString(" KD=");
    ftoa(g_param.angle_kd, b, 2);
    Uart_SendString(b);
    Uart_SendString(" VKP=");
    ftoa(g_param.speed_kp, b, 3);
    Uart_SendString(b);
    Uart_SendString(" VKI=");
    ftoa(g_param.speed_ki, b, 3);
    Uart_SendString(b);
    Uart_SendString(" VSA=");
    ftoa(g_param.speed_angle_scale, b, 4);
    Uart_SendString(b);
    Uart_SendString("\r\n");

    Uart_SendString("KZ=");
    ftoa(g_param.turn_kp, b, 3);
    Uart_SendString(b);
    Uart_SendString("MECH=");
    ftoa(g_param.mech_angle, b, 2);
    Uart_SendString(b);
    Uart_SendString(" DZIN=");
    ftoa(g_param.dz_in, b, 1);
    Uart_SendString(b);
    Uart_SendString(" DZOUT=");
    ftoa(g_param.dz_out, b, 1);
    Uart_SendString(b);
    Uart_SendString(" SP=");
    ftoa(db.target_speed, b, 1);
    Uart_SendString(b);
    Uart_SendString(" TR=");
    ftoa(db.turn_manual, b, 0);
    Uart_SendString(b);
    Uart_SendString("\r\n");
}

/* 处理一整行指令 */
static void process_line(const char *line)
{
    if (line[0] == '\0') {
        return;
    }

    if (strncmp(line, "KP=", 3) == 0) {
        g_param.angle_kp = parse_float(line + 3);
    } else if (strncmp(line, "KD=", 3) == 0) {
        g_param.angle_kd = parse_float(line + 3);
    } else if (strncmp(line, "VKP=", 4) == 0) {
        g_param.speed_kp = parse_float(line + 4);
    } else if (strncmp(line, "VKI=", 4) == 0) {
        g_param.speed_ki = parse_float(line + 4);
    } else if (strncmp(line, "VSA=", 4) == 0) {
        g_param.speed_angle_scale = parse_float(line + 4);
    } else if (strncmp(line, "KZ=", 3) == 0) {
        g_param.turn_kp = parse_float(line + 3);
    } else if (strncmp(line, "ANG=", 4) == 0) {
        g_param.mech_angle = parse_float(line + 4);
    } else if (strncmp(line, "DZIN=", 5) == 0) {
        g_param.dz_in = parse_float(line + 5);
    } else if (strncmp(line, "DZOUT=", 6) == 0) {
        g_param.dz_out = parse_float(line + 6);
    } else if (strncmp(line, "SP=", 3) == 0) {
        Balance_SetTargetSpeed(parse_float(line + 3));
    } else if (strncmp(line, "TR=", 3) == 0) {
        Balance_SetTurn(parse_float(line + 3));
    } else if (strcmp(line, "RUN=1") == 0) {
        if (Balance_RequestArm() != 1u) {
            Uart_SendString("ARM NG: tilt too far\r\n");
            return;
        }
    } else if (strcmp(line, "RUN=0") == 0) {
        Balance_Stop();
    } else if (strcmp(line, "CAL") == 0) {
        Uart_SendString("CAL... keep still 2s\r\n");
        Balance_Recalibrate();
        Uart_SendString("CAL DONE\r\n");
        return;
    } else if (strcmp(line, "INFO") == 0) {
        send_info();
        return;
    } else if (strcmp(line, "HELP") == 0) {
        send_help();
        return;
    } else {
        Uart_SendString("ERR\r\n");
        return;
    }
    Uart_SendString("OK\r\n");
}

/* 复位行缓冲并打印开机提示 */
void Protocol_Init(void)
{
    s_len          = 0u;
    s_ready        = 0u;
    s_last_tx_tick = HAL_GetTick();
    Uart_SendString("PHXC balance car ready\r\n");
    send_help();
}

/* 中断内组帧，解析交给主循环 */
void Protocol_RxByte(uint8_t b)
{
    if (b == (uint8_t)'\r') {
        return;
    }
    if (b == (uint8_t)'\n') {
        if (s_len > 0u) {
            s_line[s_len] = '\0';
            s_ready = 1u;
        }
        return;
    }
    if (s_len < (uint8_t)(PROTO_LINE_MAX - 1u)) {
        s_line[s_len++] = (char)b;
    } else {
        s_len = 0u;   /* 超长行丢弃，重新组帧 */
    }
}

/* 主循环：指令解析 + 状态提示 + 50ms 波形 */
void Protocol_Task(void)
{
    static uint8_t last_state = 0xFFu;
    static uint8_t last_arm_ready = 0u;
    Bal_Debug_t db;
    uint8_t st;
    char b[24];

    if (s_ready) {
        s_ready = 0u;
        process_line(s_line);
        s_len = 0u;
    }

    Balance_GetDebug(&db);
    st = (uint8_t)db.state;

    /* 状态变化提示 */
    if (st != last_state) {
        last_state = st;
        if (st == BAL_RUN) {
            Uart_SendString("ARMED\r\n");
        } else if (st == BAL_FALL) {
            Uart_SendString("FALL\r\n");
        } else if (st == BAL_READY) {
            Uart_SendString("READY (brake)\r\n");
        }
    }
    /* 进入启动窗口（替代原工程 LED 快闪） */
    if ((db.arm_ready != 0u) && (last_arm_ready == 0u) && (db.armed == 0u)) {
        Uart_SendString("KEEP STILL\r\n");
    }
    last_arm_ready = db.arm_ready;

    /* 50ms 波形：angle,target_angle,speed,pwm_l,pwm_r */
    if ((HAL_GetTick() - s_last_tx_tick) >= PROTO_TX_PERIOD) {
        s_last_tx_tick = HAL_GetTick();

        ftoa(db.angle, b, 2);
        Uart_SendString(b);
        Uart_SendString(",");
        ftoa(db.target_angle, b, 2);
        Uart_SendString(b);
        Uart_SendString(",");
        ftoa(db.speed, b, 1);
        Uart_SendString(b);
        Uart_SendString(",");
        ftoa((float)db.pwm[0], b, 0);
        Uart_SendString(b);
        Uart_SendString(",");
        ftoa((float)db.pwm[1], b, 0);
        Uart_SendString(b);
        Uart_SendString("\n");
    }
}
