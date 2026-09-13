#include "motor.h"
#include "tim.h"

#define PWM_PERIOD   999   /* 定时器周期，对应 100% 占空比 */

static void Motor_Left(int speed)
{
  if (speed > 0)
  {
    HAL_GPIO_WritePin(MOTOR_L_IN1_PORT, MOTOR_L_IN1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MOTOR_L_IN2_PORT, MOTOR_L_IN2_PIN, GPIO_PIN_RESET);
  }
  else if (speed < 0)
  {
    HAL_GPIO_WritePin(MOTOR_L_IN1_PORT, MOTOR_L_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_L_IN2_PORT, MOTOR_L_IN2_PIN, GPIO_PIN_SET);
  }
  else
  {
    HAL_GPIO_WritePin(MOTOR_L_IN1_PORT, MOTOR_L_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_L_IN2_PORT, MOTOR_L_IN2_PIN, GPIO_PIN_RESET);
  }
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1,
                        (uint32_t)(PWM_PERIOD * (speed > 0 ? speed : -speed) / 100));
}

static void Motor_Right(int speed)
{
  if (speed > 0)
  {
    HAL_GPIO_WritePin(MOTOR_R_IN3_PORT, MOTOR_R_IN3_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MOTOR_R_IN4_PORT, MOTOR_R_IN4_PIN, GPIO_PIN_RESET);
  }
  else if (speed < 0)
  {
    HAL_GPIO_WritePin(MOTOR_R_IN3_PORT, MOTOR_R_IN3_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_R_IN4_PORT, MOTOR_R_IN4_PIN, GPIO_PIN_SET);
  }
  else
  {
    HAL_GPIO_WritePin(MOTOR_R_IN3_PORT, MOTOR_R_IN3_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_R_IN4_PORT, MOTOR_R_IN4_PIN, GPIO_PIN_RESET);
  }
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2,
                        (uint32_t)(PWM_PERIOD * (speed > 0 ? speed : -speed) / 100));
}

void Motor_Start(void)
{
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
}

void Motor_SetSpeed(int left, int right)
{
  if (left > 100)   left = 100;
  if (left < -100)  left = -100;
  if (right > 100)  right = 100;
  if (right < -100) right = -100;

  Motor_Left(left);
  Motor_Right(right);
}

void Motor_Forward(int speed)
{
  Motor_SetSpeed(speed, speed);
}

void Motor_Backward(int speed)
{
  Motor_SetSpeed(-speed, -speed);
}

/* 原地左转：左轮反转、右轮正转 */
void Motor_TurnLeft(int speed)
{
  Motor_SetSpeed(-speed, speed);
}

/* 原地右转：左轮正转、右轮反转 */
void Motor_TurnRight(int speed)
{
  Motor_SetSpeed(speed, -speed);
}

void Motor_Stop(void)
{
  Motor_SetSpeed(0, 0);
}
