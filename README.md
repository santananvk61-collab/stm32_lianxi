# STM32 练习工程：LED 闪烁（lianxi_1）

一个基于 **STM32CubeIDE + HAL 库** 的最小工程示例：让板载 LED（PC13）以 **500ms** 周期闪烁。
适合作为 STM32 上手、GPIO 输出、HAL 基础用法的练习模板。

---

## 1. 硬件平台

| 项目 | 说明 |
| --- | --- |
| 主控芯片 | STM32F103C8Tx（ARM Cortex-M3，STM32F1 系列） |
| 常见板型 | "Blue Pill" 蓝色开发板 / 兼容核心板 |
| 板载 LED | PC13（低电平点亮，即 `GPIO_PIN_RESET` 时灯亮） |
| 调试/下载 | ST-Link（SWD 接口） |

> 若你的板子 LED 不在 PC13，或引脚接法不同，请修改 `Core/Src/main.c` 中 `HAL_GPIO_TogglePin()` 的引脚即可。

---

## 2. 开发环境

| 工具 | 说明 |
| --- | --- |
| IDE | STM32CubeIDE（Eclipse 基础上集成 CubeMX + 编译器） |
| 库 | STM32F1xx HAL 库（由 CubeMX 生成，位于 `Drivers/`） |
| 编译器 | GCC ARM Embedded（`arm-none-eabi-gcc`） |
| 烧录/调试 | ST-Link + OpenOCD（CubeIDE 内置） |

---

## 3. 工程做了什么

主循环（`Core/Src/main.c`）逻辑非常简单：

```c
while (1)
{
  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);  // 翻转 PC13 电平
  HAL_Delay(500);                          // 延时 500ms
}
```

- **时钟**：HSI 内部 8MHz 作系统时钟，未启用 PLL（见 `SystemClock_Config()`）。
- **GPIO**：PC13 配置为推挽输出（`GPIO_MODE_OUTPUT_PP`），上电默认置高（灯灭）。

> 效果：板载 LED 以约 1Hz 频率闪烁（亮 500ms / 灭 500ms）。

---

## 4. 目录结构

```
stm32_git/
├── README.md
├── .gitignore
└── lianxi_1/                         # STM32CubeIDE 工程根目录
    ├── lianxi_1.ioc                  # CubeMX 图形化配置（芯片/时钟/引脚）
    ├── .mxproject
    ├── STM32F103C8TX_FLASH.ld        # 链接脚本（Flash / RAM 内存布局）
    ├── Core/
    │   ├── Inc/                      # 头文件
    │   │   ├── main.h
    │   │   ├── stm32f1xx_hal_conf.h  # HAL 模块开关配置
    │   │   └── stm32f1xx_it.h
    │   ├── Src/                      # 源文件
    │   │   ├── main.c                # 主程序（LED 闪烁）
    │   │   ├── stm32f1xx_hal_msp.c   # 外设底层初始化（MSP）
    │   │   ├── stm32f1xx_it.c        # 中断服务函数
    │   │   ├── syscalls.c / sysmem.c # 新lib 系统调用桩
    │   │   └── system_stm32f1xx.c
    │   └── Startup/
    │       └── startup_stm32f103c8tx.s  # 启动文件（复位向量/堆栈）
    └── Drivers/
        ├── CMSIS/                    # ARM Cortex 内核接口
        └── STM32F1xx_HAL_Driver/     # ST 官方 HAL 库源码
```

---

## 5. 上手步骤

### 方式一：用 STM32CubeIDE（推荐）

1. `git clone` 本仓库到本地。
2. 打开 STM32CubeIDE → `File → Import → Existing Projects into Workspace`，选择 `lianxi_1` 目录。
3. 连接 ST-Link，点击 **Run**（或 `Project → Build` 后 `Run → Debug`）。
4. 编译、下载、运行后，板载 LED 即开始闪烁。

> 也可直接双击 `lianxi_1.ioc`，用 CubeMX 重新生成/查看配置。

### 方式二：命令行（需自行安装工具链）

```bash
# 进入工程目录
cd lianxi_1
# 用 CubeIDE 自带的 arm-none-eabi-gcc 或系统安装的 gcc 编译
# 产物在 Debug/ 目录（已被 .gitignore 忽略）
```

---

## 6. 版本控制说明

`.gitignore` 已配置好，**只提交源码，不提交编译产物和 IDE 缓存**：

- ✅ 提交：`Core/`、`Drivers/`、`*.ioc`、`*.ld`、`.mxproject` 等源文件
- ❌ 忽略：`Debug/`（编译输出）、`.settings/`、`.metadata/`、`.cproject`、`.project`、`*.launch`、`.workbuddy/` 等

如需把新写的 `.c`/`.h` 提交上传：

```bash
git add .
git commit -m "你的说明"
git push origin main
```

---

## 7. 可扩展练习

- 把 `HAL_Delay(500)` 改成不同数值，观察闪烁快慢。
- 增加按键（输入 GPIO），实现"按一下切换 LED"。
- 改用定时器（TIM）产生精确周期，替代 `HAL_Delay` 阻塞延时。
- 用串口（USART）打印 "Hello STM32"。

---

## 8. 2026-08-04 更新：OLED(I2C) 显示 DHT11 温湿度

在原有 LED 闪烁工程基础上，新增了 **0.96 寸 SSD1306 OLED（I2C）+ DHT11 温湿度** 显示功能，构成一个最小闭环：
**传感器 → MCU 处理 → 屏幕显示**。

### 8.1 新增内容

| 文件 | 作用 |
|------|------|
| `Core/Src/ssd1306.c` / `Core/Inc/ssd1306.h` | SSD1306 OLED 驱动（I2C + 1024 字节显存 + 5×7 字库） |
| `Core/Src/dht11.c` / `Core/Inc/dht11.h` | DHT11 单总线驱动（DWT 微秒级延时读 40bit 数据） |
| `Core/Src/main.c` | 主循环：每 2 秒读温湿度并刷新 OLED |
| `lianxi_1.ioc` | 启用 I2C1(PB6/PB7)、PA0 开漏上拉、72MHz 时钟等配置 |

### 8.2 硬件接线（速查）

- OLED：`VCC→5V`、`GND→GND`、`SCL→PB6`、`SDA→PB7`；I2C 地址 `0x3C`。
- DHT11：`DATA→PA0`（开漏+上拉）、`VCC→3.3V`、`GND→GND`。

### 8.3 CubeMX 关键配置

- **SYS → Debug = Serial Wire**（保留 SWD，否则烧录后 ST-Link 连不上）。
- **RCC = HSE 8MHz**；时钟树 **HSE → PLL×9 = 72MHz**；**APB1 分频 /2 = 36MHz**（F103 上限）。
- **PA0 = Open Drain + Pull-up**（DHT11 单总线）。
- **I2C1 = PB6/PB7，Standard Mode 100kHz**（杜邦线连接降速更稳）。

### 8.4 本次踩坑 & 修复（要点）

1. **SWJ 调试口被关** → 改 `__HAL_AFIO_REMAP_SWJ_NOJTAG()` 保留 SWD（救砖：BOOT0 拨 1 进 bootloader 全片擦除）。
2. **PA0 推挽** → 改 Open Drain + Pull-up。
3. **时钟 / APB1 超频** → 72MHz + APB1 /2。
4. **新 .c 不进构建** → 加入 `Debug/Core/Src/subdir.mk` 与 `Debug/objects.list`（注意 `Debug/` 被忽略，不入库）。
5. **SDA 误接 B5** → 改回 PB7，确认地址 `0x3C`。
6. **显示发虚** → 对比度 `0xCF` 降到 `0x7F`。
7. **拉长/颠倒/半屏** → 128×64 屏 COM pins 用 `0x12`（非 0x02）。
8. **字符错乱（TEMP→IFUN）** → 字库 `bit0=顶行`，`WriteChar` 用 `1<<row`（非 `1<<(7-row)`）。**字符变形优先查字库位序，与 I2C 速度无关。**

### 8.5 编译与提交

- 命令行 `make all`：**0 error / 0 warning**，生成 `lianxi_1.elf`。
- 提交：commit `527cc39`（13 个文件，+10830 / −29）。
- 完整调试记录见 **[`docs/调试笔记_OLED_DHT11_2026-08-04.md`](docs/调试笔记_OLED_DHT11_2026-08-04.md)**。

> 注：本仓库先前提交以 LED 闪烁为主（第 1–7 节）；第 8 节为 2026-08-04 新增的 OLED+DHT11 功能说明，未改动原有内容。
