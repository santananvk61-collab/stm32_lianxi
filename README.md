# stm32_git

从零学 STM32 的练习仓库。

板子是一块十几块钱的 Blue Pill（STM32F103C8Tx），从只会点灯开始，一个外设一个外设往外摸，每个外设单独一个工程，代码里中文注释写得很密——主要是给自己看的，隔几个月回来还能想起来当时为什么这么写。

## learning_projects（9 个工程）

| 工程 | 外设 | 做什么 |
|------|------|--------|
| `LINK_project` | GPIO | 三个 LED（PA3 / PA7 / PA12）用 `state` 0→6 的状态机轮流亮，先把"开时钟 → 配引脚 → 写电平"和状态机这套骨架固定下来 |
| `PWM_light_project` | TIM3 / PWM | TIM3_CH1 从 PA6 出 PWM，Prescaler=72、Period=100，靠改占空比做呼吸灯 |
| `serial_project` | USART2 | 串口收 2 字节指令（`'R'`/`'G'` 加 `'1'`/`'0'`）解析后点亮对应 LED，第一次让板子"听懂"指令 |
| `ADC_project` | ADC1 | PA5 单通道采样，轮询读值换算成电压，结果直接挂到 OLED 上 |
| `ADC_Scan_project` | ADC1 + DMA | PA4 / PA5 两通道扫描，数据交给 DMA 搬，NTC 阻值换算成温度，OLED 显示 |
| `serial_DMA_project` | USART2 + DMA | 串口的收和发都交给 DMA，CPU 不参与搬数据，靠回调拿结果 |
| `interrupt_project` | EXTI3 + SysTick | PA3 走 EXTI3 外部中断，按键消抖做成状态机塞进 SysTick 的 1ms 里跑，主循环只管闪灯 |
| `OLED` | I2C1 | PB6 / PB7 两根线点 SSD1306，自己配了 8x16 的 ASCII 字库 |
| `iwdg_project` | IWDG | 独立看门狗，1s 超时、主循环 100ms 喂一次，附"故意饿死它"实验 |

9 个都是完整的 CubeIDE 工程，`File > Import > Existing Projects into Workspace` 导进来就能编译、下载、调试。

## 硬件平台

- 主控：STM32F103C8Tx（Cortex-M3），Blue Pill
- 板载 LED：PC13，低电平点亮
- 下载调试：ST-Link（SWD）
- 开发环境：STM32CubeIDE + CubeMX（HAL 库）
