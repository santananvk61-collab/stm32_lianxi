#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H

#include "main.h"

/* HC-SR04 超声波测距：TRIG 触发，ECHO 返回高电平时间 = 距离 */

void Ultrasonic_Init(void);
float Ultrasonic_GetDistance(void);   /* 返回距离，单位厘米 */

#endif
