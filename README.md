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
