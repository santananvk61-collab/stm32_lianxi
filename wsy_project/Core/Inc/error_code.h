#ifndef __ERROR_CODE_H__
#define __ERROR_CODE_H__

#include <stdint.h>
#include "error_code.h"

/* 设备错误码:所有外设驱动统一使用此类型返回错误状态
   约定: ERR_OK(0) = 成功, 非 0 = 失败, 具体原因见枚举 */
typedef enum {
    ERR_OK = 0,              /* 成功 */
    ERR_NOT_RESPOND,         /* I2C 无应答(设备没接/地址错) */
    ERR_INVALID_DATA,        /* 数据无效(BUSY 位=1 或 CRC 错) */
    ERR_INIT_FAILED,         /* 初始化失败(ID 错/校准读失败) */
    ERR_NOT_INIT,            /* 设备未初始化就调 Read */
} DeviceError_t;

/* 把错误码转成可显示字符串,供 OLED 显示错误信息
   放在头文件内 static inline,无需额外 .c 文件,也不占用单独编译单元 */
static inline const char* ErrorToStr(DeviceError_t err)
{
    switch(err)
    {
        case ERR_OK:           return "OK";
        case ERR_NOT_RESPOND:  return "No Respond";
        case ERR_INVALID_DATA: return "Invalid Data";
        case ERR_INIT_FAILED:  return "Init Failed";
        case ERR_NOT_INIT:     return "Not Init";
        default:               return "Unknown";
    }
}

#endif
