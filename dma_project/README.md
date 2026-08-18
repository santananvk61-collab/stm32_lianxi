# dma_project — DMA 直接存储器访问

**学什么**：让数据「自己」流动，CPU 不用一个字节一个字节地搬。

## 硬件接线

| 引脚 | 功能 | 说明 |
|------|------|------|
| PA2  | USART2_TX | 接 USB 转串口的 RX（用于看现象） |

## 运行现象

串口助手（115200）每 1 秒刷出一句话——这句话是 DMA 从内存搬到串口发出去的。

## 关键知识点

- **方向** `DMA_MEMORY_TO_PERIPH`：内存 → 外设。
- **地址是否递增**：内存递增（`MINC_ENABLE`），外设不递增（`PINC_DISABLE`，就那一个数据寄存器）。
- **通道是固定的**：USART2_TX 用 DMA1 通道 7（芯片手册规定的映射，不能乱选）。
- 用 `__HAL_LINKDMA` 把 DMA 通道「挂」到串口上，`HAL_UART_Transmit_DMA` 才能用。
- 搬完后 `DMA1_Channel7_IRQHandler` 触发，交给 `HAL_DMA_IRQHandler` 收尾。

## 构建

用 STM32CubeIDE 导入本目录，或用 `arm-none-eabi-gcc` 编译 `Core/`、`Drivers/` 下的源码。
