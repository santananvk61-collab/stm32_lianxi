#ifndef __PID_H
#define __PID_H

#include "main.h"

/* 简易位置式 PID，循迹用（其实主要用到 P 项，I、D 一般设 0） */
typedef struct
{
  float Kp;
  float Ki;
  float Kd;
  float integral;        /* 积分累加 */
  int16_t last_error;    /* 上一次误差 */
  int16_t out_max;       /* 输出限幅 */
} PID_TypeDef;

void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, int16_t out_max);
int16_t PID_Calc(PID_TypeDef *pid, int16_t error);

#endif
