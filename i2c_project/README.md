# i2c_project — I2C 总线（驱动 SSD1306 OLED）

**学什么**：用两根线跟外设通信，理解「地址 + 驱动分层」。

## 硬件接线

| 引脚 | 功能 | 说明 |
|------|------|------|
| PB6  | I2C1_SCL | 接 OLED 的 SCL |
| PB7  | I2C1_SDA | 接 OLED 的 SDA |
| 3.3V / GND | 供电 | 接 OLED 的 VCC / GND |

## 运行现象

0.96 寸 SSD1306 OLED 上显示「STM32 I2C」「Count:」和一个不断递增的数字。

## 关键知识点

- I2C 只有 **SCL（时钟）+ SDA（数据）** 两根线，每个从设备有唯一地址（OLED 是 0x3C）。
- 引脚要配成 `GPIO_MODE_AF_OD`（**开漏**）：总线靠上拉电阻决定高电平，靠拉低表示低电平。
- **驱动分层**：`main.c`（高层）→ `ssd1306.c`（把「写字」翻译成 I2C 字节）→ HAL I2C（一位位送出去）。
- 便宜 OLED 模块用 **100kHz** 比 400kHz 稳（见 `MX_I2C1_Init`）。
- OLED 是「页」组织的：每 8 行像素压成 1 字节，整屏 1024 字节显存，改完一次性刷新。

## 构建

用 STM32CubeIDE 导入本目录，或用 `arm-none-eabi-gcc` 编译 `Core/`、`Drivers/` 下的源码。
