# gpio_project — GPIO 通用输入输出

**学什么**：怎么让一个引脚输出高低电平，这是所有外设的地基。

## 硬件接线

| 引脚 | 功能 | 说明 |
|------|------|------|
| PC13 | 板载 LED | Blue Pill 自带，**低电平点亮** |

## 运行现象

上电后板载 LED 以 500ms 周期闪烁。

## 关键知识点

- **通用套路四步**：①开时钟 → ②配置引脚 → ③使用 → ④`while(1)` 死循环。
- GPIO 要先 `__HAL_RCC_xxx_CLK_ENABLE()` 开时钟，否则写寄存器无效。
- `GPIO_MODE_OUTPUT_PP`（推挽）能主动输出高/低；`GPIO_MODE_OUTPUT_OD`（开漏）只能拉低。
- PC13 是低电平点亮，`GPIO_PIN_SET` 反而是「灭」，别记反。

## 构建

用 STM32CubeIDE 导入本目录，或用 `arm-none-eabi-gcc` 编译 `Core/`、`Drivers/` 下的源码。
