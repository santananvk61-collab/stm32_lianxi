# dma_project

让 DMA 替串口发数据，串口助手每秒收到一句话，CPU 不参与搬。

- 通道不能乱选：USART2_TX 固定就是 DMA1 通道 7，手册里查的映射表。
- 内存端 MINC 递增、外设端 PINC 不递增（数据寄存器就一个地址），一开始两边都开了递增，发出来的全是乱码。
- `__HAL_LINKDMA` 把通道挂到串口句柄上，之后 `HAL_UART_Transmit_DMA` 直接用。
- 发完会进 `DMA1_Channel7_IRQHandler`，真正收尾的是 `HAL_DMA_IRQHandler`。

接线：PA2 接 USB 转串口 RX。
