#ifndef __ZJ_RTC_H__
#define __ZJ_RTC_H__

#include "rtc.h"          /* CubeMX生成的, 拿到 hrtc 句柄 */
#include "error_code.h"   /* 项目错误码, 和 aht20/bmp280 风格统一 */

DeviceError_t ZJ_RTC_SetDateTime(uint16_t year, uint8_t month, uint8_t day,
                                 uint8_t hour, uint8_t min, uint8_t sec);
DeviceError_t ZJ_RTC_GetDateTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time);

#endif