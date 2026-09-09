# exti_project

按键按一下 LED 翻转一次，主循环是空的——全靠中断，第一次体会"轮询 vs 中断"的差别。

- 中断三步：配 `GPIO_MODE_IT_FALLING` → `HAL_NVIC_EnableIRQ` 开门 → 写 `EXTI1_IRQHandler`。漏第二步引脚怎么按都没反应。
- 中断里不直接写逻辑，`HAL_GPIO_EXTI_IRQHandler` 会回调 `HAL_GPIO_EXTI_Callback`，事写在回调里。
- 上拉 + 按下接地 = 下降沿触发，这个组合想通了就记住了。
- EXTI 引脚映射要开 AFIO 时钟，这个最阴，不报错就是不工作。

接线：PA1 一端接按键一端接 GND（内部上拉），PC13 板载 LED。
