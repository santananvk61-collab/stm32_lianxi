#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"

/* 电机驱动：L298N 两路，靠 PWM 调速 + IN 脚控制正反转 */

/* 启动 PWM（上电后调用一次） */
void Motor_Start(void);

/* 设置左右轮速度，speed 范围 -100~100，负数表示反转 */
void Motor_SetSpeed(int left, int right);

/* 简单动作封装 */
void Motor_Forward(int speed);    /* 前进 */
void Motor_Backward(int speed);   /* 后退 */
void Motor_TurnLeft(int speed);   /* 原地左转（差速） */
void Motor_TurnRight(int speed);  /* 原地右转（差速） */
void Motor_Stop(void);            /* 停车 */

#endif
