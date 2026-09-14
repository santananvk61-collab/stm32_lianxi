# stm32_git

从零学 STM32 的练习仓库。

板子是一块十几块钱的 Blue Pill（STM32F103C8Tx），从只会点灯开始，一个外设一个外设往外摸，每个外设单独一个工程。

注释密度不均，如实说：`ADC_project`、`ADC_Scan_project`、`OLED` 写得最细（关键配置、显存结构、电压与 NTC 温度换算都有中文说明），`interrupt_project`、`serial_DMA_project` 有一些；最早做的那两个（`LINK_project`、`serial_project`）当时没记，回头看确实是坑。

## learning_projects（9 个工程）

| 工程 | 外设 | 做什么 |
|------|------|--------|
| `LINK_project` | GPIO | 3 路 LED（PA3 / PA7 / PA12）用 `state` 0→6 的状态机轮流亮，练输出模式配置与状态机 |
| `PWM_light_project` | TIM3 / PWM | TIM3_CH1 从 PA6 出 PWM（Prescaler=72、Period=100），占空比 0→99→0 来回扫做呼吸灯 |
| `serial_project` | USART2 | 轮询方式收 2 字节指令（`'R'`/`'G'` 加 `'1'`/`'0'`）解析后点亮对应 LED |
| `ADC_project` | ADC1 | PA5 单通道轮询采样，读值换算成电压，画到 OLED 上 |
| `ADC_Scan_project` | ADC1 + DMA | PA4 / PA5 两通道扫描、DMA 搬运，NTC 阻值换算成温度，OLED 显示 |
| `serial_DMA_project` | USART2 + DMA | 串口收发全交给 DMA，CPU 不搬数据，靠接收回调取结果 |
| `interrupt_project` | EXTI3 + SysTick | PA3 走 EXTI3 外部中断，按键消抖做成状态机塞进 SysTick 的 1ms 里，主循环只管闪灯 |
| `OLED` | I2C1 | PB6 / PB7 两根线点 SSD1306，页缓冲整屏刷新，自写画点 / 画线 / 字符串，配 8×16 ASCII 字库 |
| `iwdg_project` | IWDG | 独立看门狗，Prescaler=8 / Reload=4095（LSI≈40kHz 下约 0.8s 超时），主循环 100ms 喂一次 |

9 个工程都带完整源码（`Core` / `Drivers` / 启动文件 / 链接脚本）和各自的 `.ioc` 配置，另外都带 CubeIDE 的工程文件：`File > Import > Existing Projects into Workspace` 导进来就能编译、下载；也可以直接用 STM32CubeMX 打开 `.ioc` 重新生成。

## 硬件平台

- 主控：STM32F103C8Tx（Cortex-M3），Blue Pill
- 板载 LED：PC13，低电平点亮
- 下载调试：ST-Link（SWD）
- 开发环境：STM32CubeIDE + CubeMX（HAL 库）
