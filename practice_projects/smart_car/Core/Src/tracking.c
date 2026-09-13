#include "tracking.h"
#include "motor.h"
#include "pid.h"

static PID_TypeDef pid;

void Tracking_Init(void)
{
  /* 循迹只用 P 项就够了，I、D 先设 0 */
  PID_Init(&pid, 20.0f, 0.0f, 0.0f, 45);
}

/* 读一路传感器，压到黑线返回 1，否则 0 */
static uint8_t ReadOne(GPIO_TypeDef *port, uint16_t pin)
{
  return (HAL_GPIO_ReadPin(port, pin) == TRACK_ON_LINE_LEVEL) ? 1 : 0;
}

/*
 * 算偏差。传感器从左到右是 L2 L1 R1 R2。
 * 用加权和：右边压线为正，左边压线为负，范围大概 -3 ~ +3。
 * error > 0 说明线偏右了，车要往右修；error < 0 线偏左，往左修。
 */
int8_t Tracking_GetError(void)
{
  int8_t L2 = ReadOne(TRACK_L2_PORT, TRACK_L2_PIN);
  int8_t L1 = ReadOne(TRACK_L1_PORT, TRACK_L1_PIN);
  int8_t R1 = ReadOne(TRACK_R1_PORT, TRACK_R1_PIN);
  int8_t R2 = ReadOne(TRACK_R2_PORT, TRACK_R2_PIN);

  return (R1 + 2 * R2) - (L1 + 2 * L2);
}

void Tracking_LineFollow(int base_speed)
{
  int8_t error = Tracking_GetError();

  /* 四路都没压线 = 完全丢线，停车防止冲出去 */
  if (error == 0)
  {
    uint8_t on_line =
        ReadOne(TRACK_L2_PORT, TRACK_L2_PIN) |
        ReadOne(TRACK_L1_PORT, TRACK_L1_PIN) |
        ReadOne(TRACK_R1_PORT, TRACK_R1_PIN) |
        ReadOne(TRACK_R2_PORT, TRACK_R2_PIN);
    if (on_line == 0)
    {
      Motor_Stop();
      return;
    }
  }

  int16_t turn = PID_Calc(&pid, error);
  /* 线在右边 -> 左轮加速、右轮减速，让车头往右摆 */
  int left  = base_speed + turn;
  int right = base_speed - turn;
  Motor_SetSpeed(left, right);
}
