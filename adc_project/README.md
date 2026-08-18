# adc_project — ADC 模数转换

**学什么**：把模拟电压变成数字量，理解「12 位 / 基准电压 / 换算」。

## 硬件接线

| 引脚 | 功能 | 说明 |
|------|------|------|
| PA0  | ADC1_IN0 | 接一个可变电压（电位器分压，0~3.3V） |
| PC13 | 板载 LED | 低电平点亮 |

## 运行现象

LED 闪烁速度随 PA0 电压变化：电压越高，闪得越快。

## 关键知识点

- **换算公式**：`电压(mV) = 读数 × 3300 / 4095`（12 位 ADC 读数范围 0~4095，对应 0~3.3V）。
- 采样流程：`HAL_ADC_Start` → `HAL_ADC_PollForConversion`（等完成）→ `HAL_ADC_GetValue`（取数）→ `HAL_ADC_Stop`。
- `ADC_SAMPLETIME_239CYCLES_5`：采样时间越长，读数越稳。
- ADC 时钟默认 PCLK2/2 = 4MHz（复位默认值，在合法范围 0.6~14MHz 内）。

## 构建

用 STM32CubeIDE 导入本目录，或用 `arm-none-eabi-gcc` 编译 `Core/`、`Drivers/` 下的源码。
