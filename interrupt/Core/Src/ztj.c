/* 按键扫描：状态机消抖实现 */

#include "ztj.h"

/* 按键消抖状态机：4 个状态 */
typedef enum
{
  KEY_IDLE,             /* 空闲，等待按键 */
  KEY_DEBOUNCE_PRESS,   /* 按下消抖，累计连续按下次数 */
  KEY_PRESSED,          /* 已确认按下 */
  KEY_DEBOUNCE_RELEASE  /* 松开消抖，累计连续松开次数 */
} KeyState;

static KeyState key_state = KEY_IDLE;
static uint8_t  debounce_cnt = 0;

#define KEY_DEBOUNCE_MAX   5                /* 连续 5 次采样才确认，约 5ms */
#define KEY_PRESSED_LEVEL  GPIO_PIN_SET     /* 按下 = 高电平（PA3 下拉，按键接 3.3V） */

/**
  * @brief  按键扫描（状态机消抖），必须周期性调用（本工程在 SysTick 1ms 中断里调用）
  * @note   一次有效的“按下→松开”只产生一次返回值 1
  * @retval 1：确认按下（产生一次事件）；0：无事件
  */
uint8_t Key_Scan(void)
{
  uint8_t level = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);

  switch (key_state)
  {
  case KEY_IDLE:
    if (level == KEY_PRESSED_LEVEL)          /* 第一次看到按下，不急着信 */
    {
      debounce_cnt = 1;
      key_state = KEY_DEBOUNCE_PRESS;
    }
    break;

  case KEY_DEBOUNCE_PRESS:
    if (level == KEY_PRESSED_LEVEL)
    {
      if (++debounce_cnt >= KEY_DEBOUNCE_MAX)  /* 连续 N 次都按下 */
      {
        key_state = KEY_PRESSED;
        return 1u;                             /* 确认按下：产生一次事件 */
      }
    }
    else
    {
      key_state = KEY_IDLE;                    /* 抖动，回空闲重来 */
    }
    break;

  case KEY_PRESSED:
    if (level != KEY_PRESSED_LEVEL)            /* 看到松开，进入松开消抖 */
    {
      debounce_cnt = 1;
      key_state = KEY_DEBOUNCE_RELEASE;
    }
    break;

  case KEY_DEBOUNCE_RELEASE:
    if (level != KEY_PRESSED_LEVEL)
    {
      if (++debounce_cnt >= KEY_DEBOUNCE_MAX)
      {
        key_state = KEY_IDLE;                  /* 确认松开 */
      }
    }
    else
    {
      key_state = KEY_PRESSED;                 /* 松开过程抖动，回到按下 */
    }
    break;

  default:
    key_state = KEY_IDLE;
    break;
  }
  return 0u;
}