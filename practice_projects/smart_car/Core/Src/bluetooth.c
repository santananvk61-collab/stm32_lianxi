#include "bluetooth.h"
#include "usart.h"
#include "motor.h"

static uint8_t rx_byte = 0;        /* 接收缓冲区（必须一直存在，不能放栈上） */
volatile uint8_t bt_flag = 0;      /* 收到新指令的标志 */
volatile uint8_t bt_cmd = 0;       /* 收到的指令字符 */

/* 上电后开始接收第一个字节 */
void Bluetooth_Init(void)
{
  HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

/* 接收完成回调：每收到一个字节，串口中断就会走到这里 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    bt_cmd = rx_byte;
    bt_flag = 1;
    /* 重新开启接收，等下一个字节 */
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
  }
}

/*
 * 手机蓝牙串口 App 发单个字符来控制：
 *   F/f 前进  B/b 后退  L/l 左转  R/r 右转  S/s 停止
 *   A/a 切到自动循迹模式（这个在 main 里处理）
 */
void Bluetooth_Process(void)
{
  if (bt_flag == 0)
  {
    return;
  }
  bt_flag = 0;

  switch (bt_cmd)
  {
    case 'F':
    case 'f':
      Motor_Forward(60);
      break;
    case 'B':
    case 'b':
      Motor_Backward(60);
      break;
    case 'L':
    case 'l':
      Motor_TurnLeft(45);
      break;
    case 'R':
    case 'r':
      Motor_TurnRight(45);
      break;
    case 'S':
    case 's':
      Motor_Stop();
      break;
    default:
      break;
  }
}
