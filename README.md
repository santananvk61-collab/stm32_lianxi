# stm32_git

从零学 STM32 的练习仓库。

板子是一块十几块钱的 Blue Pill（STM32F103C8Tx），从只会点灯开始，一个外设一个外设往外摸，每个外设单独一个工程，代码里中文注释写得很密——主要是给自己看的，隔几个月回来还能想起来当时为什么这么写。

## learning_projects（9 个学习工程）

| 工程 | 内容 |
|------|------|
| `gpio_project` | 点灯 + 流水灯，固定"开时钟→配引脚→使用"的套路 |
| `pwm_project` | TIM 时基 + PWM 呼吸灯，占空比调亮度 |
| `uart_project` | 串口回声，第一次跟电脑"对话" |
| `adc_project` | 电位器采样，读数换算电压控制 LED 闪速 |
| `dma_project` | DMA 替串口发数据，CPU 不动手 |
| `exti_project` | 外部中断按键翻转 LED，主循环是空的 |
| `i2c_project` | 两根线点亮 SSD1306 OLED |
| `onewire_project` | 单总线读 DHT11，微秒级时序 + DWT 延时 |
| `iwdg_project` | 独立看门狗，1s 超时喂狗，附"故意饿死它"实验 |

每个工程目录下有 README，记的是接线、现象和自己当时踩的坑（`Prescaler` 的 +1、AFIO 时钟、OLED 降速到 100kHz、DMA 递增开错边……）。对我自己来说坑比代码值钱。

## 硬件平台

- 主控：STM32F103C8Tx（Cortex-M3），Blue Pill
- 板载 LED：PC13，低电平点亮
- 下载调试：ST-Link（SWD）
- 开发环境：STM32CubeIDE + HAL 库
