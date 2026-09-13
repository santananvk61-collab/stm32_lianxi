
#ifndef INC_ZJ_RTC_H_
#define INC_ZJ_RTC_H_

#include "stm32f1xx_hal.h"
#include "rtc.h"
#include "time.h"

HAL_StatusTypeDef zj_RTC_SetTime(struct tm* zj_time);

struct tm* zj_RTC_GetTime(void);

void zj_RTC_Init(void);

#endif /* INC_ZJ_RTC_H_ */
