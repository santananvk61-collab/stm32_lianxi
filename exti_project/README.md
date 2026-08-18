# exti_project — EXTI 外部中断

**学什么**：让引脚电平变化「打断」CPU，理解中断 vs 轮询。

## 硬件接线

| 引脚 | 功能 | 说明 |
|------|------|------|
| PA1  | 按键输入 | 一端接 PA1，一端接 GND，内部上拉 |
| PC13 | 板载 LED | 低电平点亮 |

## 运行现象

每按一次按键，板载 LED 翻转一次。主循环里什么都没做——全靠中断。

## 关键知识点

- **轮询**是 CPU 反复问「变了吗」，**中断**是引脚主动说「我变了」。
- 中断三步：①配成 `GPIO_MODE_IT_FALLING` → ②`HAL_NVIC_SetPriority/EnableIRQ` 开门 → ③写 `EXTI1_IRQHandler`。
- `EXTI1_IRQHandler` 里调 `HAL_GPIO_EXTI_IRQHandler`，它最终回调 `HAL_GPIO_EXTI_Callback`（我们在这里做事）。
- 下降沿 + 上拉：按下时引脚从高变低 = 下降沿。
- 外部中断的引脚映射需要 AFIO 时钟（`HAL_MspInit` 里已开）。

## 构建

用 STM32CubeIDE 导入本目录，或用 `arm-none-eabi-gcc` 编译 `Core/`、`Drivers/` 下的源码。
