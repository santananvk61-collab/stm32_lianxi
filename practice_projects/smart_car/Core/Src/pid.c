#include "pid.h"

void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, int16_t out_max)
{
  pid->Kp = kp;
  pid->Ki = ki;
  pid->Kd = kd;
  pid->integral = 0.0f;
  pid->last_error = 0;
  pid->out_max = out_max;
}

int16_t PID_Calc(PID_TypeDef *pid, int16_t error)
{
  float out;

  pid->integral += error;                    /* 积分（循迹一般不用，Ki 设 0） */
  int16_t diff = error - pid->last_error;    /* 微分 */
  pid->last_error = error;

  out = pid->Kp * error + pid->Ki * pid->integral + pid->Kd * diff;

  /* 输出限幅，防止转向修正量过大 */
  if (out > pid->out_max)  out = pid->out_max;
  if (out < -pid->out_max) out = -pid->out_max;

  return (int16_t)out;
}
