# stm32_git — STM32 学习与练习仓库

本仓库用于记录 STM32F103C8Tx（Blue Pill）的开发练习，包含若干独立的 STM32CubeIDE 工程，以及对应的调试记录和学习笔记。工程基于 HAL 库，由 CubeMX 生成骨架。

## 仓库结构

| 目录 | 内容 |
|------|------|
| `lianxi_1/` | 基础工程：板载 LED 闪烁；扩展 OLED(I2C) 显示 DHT11 温湿度 |
| `LINK/` | 流水灯状态机练习（PA7/PA12/PA3 三路 LED） |
| `interrupt/` | 外部中断（EXTI3）+ SysTick 状态机按键消抖练习 |
| `key/` | 按键消抖练习（轮询 + 软件消抖） |
| `docs/` | 调试笔记与知识整理 |
| `notes/` | 学习笔记 |

## 硬件平台

| 项目 | 说明 |
|------|------|
| 主控芯片 | STM32F103C8Tx（ARM Cortex-M3，STM32F1 系列） |
| 开发板 | Blue Pill 及兼容核心板 |
| 板载 LED | PC13，低电平点亮 |
| 调试与下载 | ST-Link（SWD 接口） |
| 开发环境 | STM32CubeIDE（CubeMX + HAL 库 + arm-none-eabi-gcc） |

## 工程说明

### lianxi_1

基础练习工程，主循环翻转板载 LED（PC13，500ms 周期）。在此基础上扩展了 0.96 寸 SSD1306 OLED（I2C，PB6/PB7）和 DHT11 温湿度传感器（单总线，PA0），每 2 秒读取一次并刷新屏幕。

### LINK

流水灯状态机练习。PA7、PA12、PA3 三路 LED 按 0~6 七个状态循环点亮，每个状态停留 100ms。

### interrupt

外部中断按键练习。PA3 配置为 EXTI3 上升沿中断，按键状态机消抖在 SysTick 中断中每 1ms 调用一次 `Key_Scan()`（见 `Core/Src/ztj.c`），确认一次有效按下后翻转 PA12；主循环中 PA7 以 100ms 周期闪烁。

### key

按键消抖练习。PA8（上拉输入）与 PA3（下拉输入）两个按键，主循环轮询并做 20ms 软件消抖，控制 PA12、PA7 两路 LED。

## 导入与构建

1. 克隆仓库到本地。
2. 使用 STM32CubeIDE 的 `File → Import → Existing Projects into Workspace` 导入对应工程目录。
3. 连接 ST-Link 后编译、下载、运行。

工程由 CubeMX 生成，也可直接打开 `*.ioc` 重新生成代码。命令行构建需自行安装 `arm-none-eabi-gcc` 工具链，产物输出在 `Debug/`。

## 版本控制

仓库只跟踪源码与工程配置（`Core/`、`Drivers/`、`*.ioc`、`*.ld`、`.mxproject`），忽略编译产物（`Debug/`）与 IDE 缓存（`.settings/`、`.metadata/`、`.cproject`、`.project`、`*.launch` 等），规则见 `.gitignore`。

注意：CubeIDE 的 make 构建不会自动把新添加的源文件编入，需手动更新 `Debug/Core/Src/subdir.mk` 与 `Debug/objects.list`。该改动位于被忽略的 `Debug/` 目录，不会进入版本库，换环境后需重新处理。

## 相关文档

- `docs/调试笔记_OLED_DHT11_2026-08-04.md` — OLED 与 DHT11 调试记录
- `docs/按键消抖与排错知识整理_2026-08-11.md` — 按键排错与消抖方案整理
- `notes/学习笔记_位运算_2026-08-06.md` — 位运算学习笔记
- `notes/位运算_一页笔记_易错点清单.md` — 位运算易错点速记