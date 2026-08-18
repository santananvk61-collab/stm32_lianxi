/*
 * stm32f1xx_hal_msp.c —— MSP 初始化
 *
 * 本项目用 I2C1（PB6=SCL, PB7=SDA）。I2C 要求引脚配成「复用开漏」，
 * 因为 I2C 总线靠上拉电阻决定高电平、靠拉低表示低电平（线与）。
 */
#include "main.h"

void HAL_MspInit(void)
{
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
}

/* I2C 的底层初始化：HAL_I2C_Init() 会回头调用它 */
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (hi2c->Instance == I2C1)
  {
    __HAL_RCC_I2C1_CLK_ENABLE();    /* 给 I2C1 开时钟 */
    __HAL_RCC_GPIOB_CLK_ENABLE();   /* PB6/PB7 在 GPIOB 上 */

    /* PB6(SCL) + PB7(SDA)：复用开漏（I2C 的标配） */
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;   /* 复用功能 + 开漏 */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
  if (hi2c->Instance == I2C1)
  {
    __HAL_RCC_I2C1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);
  }
}
