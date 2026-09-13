#ifndef __TRACKING_H
#define __TRACKING_H

#include "main.h"

/* 循迹模块：读 4 路 TCRT5000，算偏差，PID 修正后驱动电机 */

void Tracking_Init(void);
int8_t Tracking_GetError(void);              /* 返回偏差：0 居中，负偏左，正偏右 */
void Tracking_LineFollow(int base_speed);    /* 循迹主逻辑 */

#endif
