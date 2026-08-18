/*
 * i2c_project —— 第 7 个外设：I2C（同步串行总线）
 *
 * 学习目标：用两根线（SCL 时钟 + SDA 数据）跟一堆外设通信。
 * 每个 I2C 从设备都有唯一地址，主机按地址点名通信，非常省引脚。
 *
 * 本程序做的事：
 *   用 I2C1（PB6=SCL, PB7=SDA）驱动一块 0.96 寸 SSD1306 OLED 屏，
 *   显示一句话和一个不断递增的计数器。
 *
 * 分层结构（重要，体会「驱动分层」）：
 *   main.c      —— 只调用高层函数（Init / WriteString / UpdateScreen）
 *   ssd1306.c   —— OLED 驱动：把「写字符」翻译成 I2C 字节流
 *   HAL I2C     —— 芯片库：负责在总线上一位一位地把字节送出去
 */
#include "main.h"
#include "ssd1306.h"

I2C_HandleTypeDef hi2c1;

void SystemClock_Config(void);
static void MX_I2C1_Init(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_I2C1_Init();

  SSD1306_Init();   /* 上电初始化 OLED（内部发一串配置命令） */

  int count = 0;

  while (1)
  {
    /* 每次整屏重画：先清显存，再写内容，最后一次性刷到屏幕 */
    SSD1306_Clear();

    SSD1306_SetCursor(0, 0);
    SSD1306_WriteString("STM32 I2C");

    SSD1306_SetCursor(0, 16);
    SSD1306_WriteString("Count:");

    SSD1306_SetCursor(0, 32);
    SSD1306_WriteInt(count++);   /* 数字在涨，说明程序一直在跑 */

    SSD1306_UpdateScreen();      /* 把显存刷到屏幕（不调这句看不到变化） */
    HAL_Delay(500);
  }
}

/* 系统时钟：内部 HSI 8MHz */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    Error_Handler();

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    Error_Handler();
}

/* I2C1 初始化：100kHz（便宜 OLED 模块用 100k 比 400k 稳） */
static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;                        /* 100kHz */
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;                            /* 本机是主机，不需要地址 */
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;   /* 7 位地址 */
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    Error_Handler();
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
