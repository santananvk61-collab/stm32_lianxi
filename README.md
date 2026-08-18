# stm32_git — STM32 学习仓库

这是我从零学 STM32 时留下的练习仓库。

用的是一块十几块钱的 Blue Pill（STM32F103C8Tx，Cortex-M3）。一开始我只会点亮一颗 LED，后来陆陆续续把 GPIO、定时器、串口、ADC、PWM、中断、DMA、I2C、单总线这些外设都摸了一遍。每个外设单独开了一个工程，代码里写满了中文注释，方便以后回头看——也方便有同样困惑的人照着走。

工程按用途分成了两个文件夹：

- **`learning_projects/`（学习工程）**——「9 大外设」的标准学习工程，每个只学一个外设，代码精简、注释齐全，适合照着入门。
- **`practice_projects/`（练习工程）**——动手做的综合练习，功能更杂、串口变体较多，风格更随意。

## learning_projects（学习工程）

嵌入式入门有个规律：先跑通「点灯」这个最经典的 Hello World，再一个外设一个外设往外扩。这里每个工程只学一个外设：

- **GPIO**（`gpio_project`）—— 点灯。套路就四步：开时钟 → 配置引脚 → 使用 → 死循环。
- **ADC**（`adc_project`）—— 把模拟电压变成数字，转出来的值直接控制 LED 闪多快。
- **UART**（`uart_project`）—— 串口，让 MCU 跟电脑聊天。做的是「回声」：敲什么就回什么。
- **PWM**（`pwm_project`）—— 用方波调亮度，做了个呼吸灯。
- **EXTI**（`exti_project`）—— 外部中断，引脚电平一变就打断 CPU，按键翻转 LED。
- **DMA**（`dma_project`）—— 让数据自己流动，CPU 不用一个字节一个字节搬。这里让 DMA 替串口发数据。
- **I2C**（`i2c_project`）—— 两根线驱动一块 0.96 寸 OLED 屏。
- **单总线**（`onewire_project`）—— 一根线读 DHT11 温湿度。

> 每个工程目录下还有一份自己的 README，写了接线方法和运行现象；源码注释里也尽量把「为什么这么写」讲清楚了。

## practice_projects（练习工程）

这几个是一个想法一个工程敲出来的综合练习，功能更杂一些：

- `interrupt_project` —— 外部中断（EXTI3）+ SysTick 状态机消抖
- `key_project` —— 轮询 + 20ms 软件消抖，控制两路 LED
- `lianxi_1_project` —— 板载 LED + OLED 显示 DHT11 温湿度
- `LINK_project` —— 三路 LED 流水灯状态机
- `serial_project` —— 串口指令控制 LED
- `serial_DMA_project` —— 串口 + DMA
- `chuankou_zhonduang_project` —— 串口中断
- `TIM_project` —— TIM4 定时器基础

## 硬件平台

- 主控：STM32F103C8Tx（Cortex-M3）
- 开发板：Blue Pill（蓝色小板）
- 板载 LED：PC13，低电平点亮
- 下载调试：ST-Link（SWD）
- 开发环境：STM32CubeIDE + HAL 库

## 笔记

除了代码，调试时踩过的坑也记在 `docs/` 和 `notes/` 里了——比如 OLED 字符乱码、I2C 跑不稳降速、字库位序不对导致整字颠倒之类的。踩坑记录往往比代码本身更有用。
