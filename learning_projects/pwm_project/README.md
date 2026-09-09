# pwm_project

呼吸灯。主要搞清楚了三件事：

- 频率 = 8MHz / 8 / 1000 = 1kHz，`Prescaler = 7` 寄存器存的是 7、实际分频 8（+1），这个 +1 忘了就差 8 倍。
- 亮度 = 占空比 = Pulse / 周期，改 `__HAL_TIM_SET_COMPARE` 就行。
- `HAL_TIM_PWM_Start` 忘了调，配置全对但就是没波形，查了一晚上。

接线：PA6（TIM3_CH1）串电阻接 LED 到 GND。
