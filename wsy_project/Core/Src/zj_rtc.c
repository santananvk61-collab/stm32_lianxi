#include "zj_rtc.h"

/* 设置日期时间(年=2000~2099, 其余正常范围)
   顺序必须是: 先 SetDate 后 SetTime, 与ST官方F1例程一致 */
DeviceError_t ZJ_RTC_SetDateTime(uint16_t year, uint8_t month, uint8_t day,
                                 uint8_t hour, uint8_t min, uint8_t sec)
{
    RTC_DateTypeDef d = {0};
    RTC_TimeTypeDef t = {0};

    d.Year  = (uint8_t)(year - 2000);   /* 结构体里存的是 0~99 */
    d.Month = month;
    d.Date  = day;
    d.WeekDay = RTC_WEEKDAY_MONDAY;     /* 没有真实日历, 星期是摆设, 随便填 */

    t.Hours   = hour;
    t.Minutes = min;
    t.Seconds = sec;

    if(HAL_RTC_SetDate(&hrtc, &d, RTC_FORMAT_BIN) != HAL_OK) return ERR_INIT_FAILED;
    if(HAL_RTC_SetTime(&hrtc, &t, RTC_FORMAT_BIN) != HAL_OK) return ERR_INIT_FAILED;
    return ERR_OK;
}

/* 读日期时间
   !!! 关键: 必须先 GetTime 再 GetDate, 成对调用, 顺序不能反 !!! */
DeviceError_t ZJ_RTC_GetDateTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time)
{
    if(HAL_RTC_GetTime(&hrtc, time, RTC_FORMAT_BIN) != HAL_OK) return ERR_NOT_RESPOND;
    if(HAL_RTC_GetDate(&hrtc, date, RTC_FORMAT_BIN) != HAL_OK) return ERR_NOT_RESPOND;
    return ERR_OK;
}