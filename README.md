# stm32_git — STM32 学习与练习仓库

本仓库用于记录 STM32F103C8Tx（Blue Pill）的嵌入式外设学习与开发练习，包含 9 个外设学习工程、若干综合练习工程，以及对应的调试记录和学习笔记。工程基于 HAL 库，由 STM32CubeMX / STM32CubeIDE 生成骨架。

## 外设学习工程（9 个）

与简历中的「STM32 嵌入式学习项目集」一一对应：

| 外设 | 目录 | 练习内容 | 进度 |
|------|------|----------|------|
| GPIO | `gpio_project` | LED 闪烁（开时钟→配置→使用→死循环四步套路） | 已完成 |
| TIM | `TIM_project` | 定时器时基、更新中断 | 已有基础 |
| ADC | `adc_project` | 采样与电压换算（电压控制 LED 闪烁快慢） | 已完成 |
| UART | `uart_project` | 串口收发、回声测试（115200 8N1） | 已完成 |
| PWM | `pwm_project` | TIM3 PWM 输出、呼吸灯 | 已完成 |
| EXTI | `exti_project` | 外部中断按键翻转 LED | 已完成 |
| DMA | `dma_project` | 内存到串口的 DMA 搬运 | 已完成 |
| I2C | `i2c_project` | I2C 驱动 SSD1306 OLED | 已完成 |
| 单总线 | `onewire_project` | DHT11 温湿度读取与校验 | 已完成 |

> 每个外设学习工程目录下都有一份独立 `README.md`，说明学习目标、硬件接线、运行现象和关键知识点；源码内也带了逐行中文注释。

## 综合练习工程

| 目录 | 内容 |
|------|------|
| `lianxi_1_project/` | 基础工程：板载 LED 闪烁；扩展 OLED(I2C) 显示 DHT11 温湿度 |
| `LINK_project/` | 流水灯状态机练习（PA7/PA12/PA3 三路 LED） |
| `interrupt_project/` | 外部中断（EXTI3）+ SysTick 状态机按键消抖练习 |
| `key_project/` | 按键消抖练习（轮询 + 20ms 软件消抖） |
| `serial_project/` | USART 串口收发，指令控制三路 LED |
| `TIM_project/` | TIM4 定时器基础配置练习 |

## 硬件平台

| 项目 | 说明 |
|------|------|
| 主控芯片 | STM32F103C8Tx（ARM Cortex-M3，STM32F1 系列） |
| 开发板 | Blue Pill 及兼容核心板 |
| 板载 LED | PC13，低电平点亮 |
| 调试与下载 | ST-Link（SWD 接口） |
| 开发环境 | STM32CubeIDE（CubeMX + HAL 库 + arm-none-eabi-gcc） |

## 工程说明

### lianxi_1_project

基础练习工程，主循环翻转板载 LED（PC13，500ms 周期）。在此基础上扩展了 0.96 寸 SSD1306 OLED（I2C，PB6/PB7）和 DHT11 温湿度传感器（单总线，PA0），每 2 秒读取一次并刷新屏幕。

### LINK_project

流水灯状态机练习。PA7、PA12、PA3 三路 LED 按 0~6 七个状态循环点亮，每个状态停留 100ms。

### interrupt_project

外部中断按键练习。PA3 配置为 EXTI3 上升沿中断，按键状态机消抖在 SysTick 中断中每 1ms 调用一次 `Key_Scan()`（见 `Core/Src/ztj.c`），确认一次有效按下后翻转 PA12；主循环中 PA7 以 100ms 周期闪烁。

### key_project

按键消抖练习。PA8（上拉输入）与 PA3（下拉输入）两个按键，主循环轮询并做 20ms 软件消抖，控制 PA12、PA7 两路 LED。

### serial_project

USART2 串口通信练习。115200 波特率收发，接收两字节指令（如 `R1` / `G0` / `Y1`）控制三路 LED 亮灭，练习串口协议与字节解析。

### TIM_project

TIM4 定时器基础练习。配置预分频 7199、自动重载 65535，练习时基配置与中断/更新事件。

## 导入与构建

1. 克隆仓库到本地。
2. 使用 STM32CubeIDE 的 `File → Import → Existing Projects into Workspace` 导入对应工程目录。
3. 连接 ST-Link 后编译、下载、运行。

工程由 CubeMX 生成，也可直接打开 `*.ioc` 重新生成代码。命令行构建需自行安装 `arm-none-eabi-gcc` 工具链，产物输出在 `Debug/`。

其中 9 个外设学习工程为「纯源码」版本：含 `Core/`、`Drivers/`、链接脚本 `.ld` 与 `.mxproject`，可被 STM32CubeIDE 直接作为已有工程导入构建；为保持简洁未包含 `.ioc`（如需 CubeMX 图形化配置，可用相同引脚参数自行新建 `.ioc`）。综合练习工程则保留了 `.ioc`。

## 版本控制

仓库只跟踪源码与工程配置（`Core/`、`Drivers/`、`*.ioc`、`*.ld`、`.mxproject`），忽略编译产物（`Debug/`）与 IDE 缓存（`.settings/`、`.metadata/`、`.cproject`、`.project`、`*.launch` 等），规则见 `.gitignore`。空目录通过 `.gitkeep` 占位提交。

注意：CubeIDE 的 make 构建不会自动把新添加的源文件编入，需手动更新 `Debug/Core/Src/subdir.mk` 与 `Debug/objects.list`。该改动位于被忽略的 `Debug/` 目录，不会进入版本库，换环境后需重新处理。
