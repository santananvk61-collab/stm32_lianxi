#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"

/* 电机驱动：L298N 两路（四轮车左右各 2 个轮子并联），PWM 调速 + IN 脚控制正反转 */

void Motor_Start(void);
void Motor_SetSpeed(int left, int right);   /* -100~100，负数反转 */
void Motor_Forward(int speed);
void Motor_Backward(int speed);
void Motor_TurnLeft(int speed);
void Motor_TurnRight(int speed);
void Motor_Stop(void);

#endif
