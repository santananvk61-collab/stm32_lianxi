# stm32_git

STM32 工程集，每个外设一个独立工程。

| 目录 | 内容 |
|------|------|
| `learning_projects/` | 9 个外设练习工程 |
| `wsy_project/` | 温湿度气压采集显示 |
| `phxc_project/` | 两轮平衡小车 |
| `hardware/` | 对应板子的立创EDA Pro 工程 |

## learning_projects

| 工程 | 外设 | 做什么 |
|------|------|--------|
| `LINK_project` | GPIO | 3 路 LED（PA3/PA7/PA12）按状态机轮流亮 |
| `PWM_light_project` | TIM3_CH1 | PA6 出 10kHz PWM（PSC=71/ARR=99），呼吸灯 |
| `serial_project` | USART2 | 115200 轮询收指令，解析后控制 LED |
| `ADC_project` | ADC1 | PA5 轮询采样，读值换算电压画到 OLED |
| `ADC_Scan_project` | ADC1 + DMA | 4 路规则通道 DMA 搬运，VREFINT 反推参考电压，NTC 换算温度 |
| `serial_DMA_project` | USART2 + DMA | 收发走 DMA，回调取结果 |
| `interrupt_project` | EXTI3 | 按键外部中断，消抖状态机跑在 SysTick 1ms |
| `OLED` | I2C1 | PB6/PB7 驱动 SSD1306，页缓冲整屏刷新，自写画点/画线/字符串 |
| `iwdg_project` | IWDG | Prescaler=8/Reload=4095，约 0.8s 超时，100ms 喂狗 |

## 其他工程

- `wsy_project/`：STM32F103C8T6 + AHT20 + BMP280 + SSD1306，三行显示温度/湿度/气压，I2C/RTC/DMA/USART 分文件
- `phxc_project/`：STM32F401CCU6 + ICM-42670-L（SPI1）+ 双 AT8236 + AB 相编码器，1ms 定周期姿态解算与串级 PID
- `hardware/`：两块板子的立创EDA Pro 工程（`temp-humidity-board.epro2`、`balance-car.epro2`）

## 硬件平台

- F103C8T6（Blue Pill）：PC13 LED 低电平点亮，ST-Link SWD
- F401CCU6：四层板 96×59mm
