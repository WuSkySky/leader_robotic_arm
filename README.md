# Leader Robotic Arm

## 串口控制帧

串口发送一帧固定 38 字节，所有 `float` 按 MCU 小端格式直接发送。

| 偏移 | 字节数 | 说明 |
| ---- | ------ | ---- |
| 0 | 1 | 帧头 `0xAA` |
| 1 | 4 | ID0 机械臂位置 `float` |
| 5 | 4 | ID1 机械臂位置 `float` |
| 9 | 4 | ID2 机械臂位置 `float` |
| 13 | 4 | ID3 机械臂位置 `float` |
| 17 | 4 | ID4 机械臂位置 `float` |
| 21 | 4 | ID5 机械臂位置 `float` |
| 25 | 4 | 底盘 `vx`，前后速度，单位 m/s，最大 `4.0` |
| 29 | 4 | 底盘 `vy`，左右平移速度，单位 m/s，最大 `4.0` |
| 33 | 4 | 底盘 `vw`，左右旋转角速度，单位 rad/s，最大 `1.57` |
| 37 | 1 | 帧尾 `0x55` |

## 摇杆映射

| 摇杆 ID | 控制量 | 说明 |
| ------- | ------ | ---- |
| 3 | `vx` | 前后移动 |
| 2 | `vy` | 左右平移 |
| 0 | `vw` | 左右旋转 |

## 按键配置

按键使用 `PE2~PE11`，共 10 个。硬件按下接 GND，因此 CubeMX 中必须配置为内部上拉和双边沿 EXTI。

| 按键 ID | 引脚 | 按下电平 |
| ------- | ---- | -------- |
| KEY0 | PE2 | 低电平 |
| KEY1 | PE3 | 低电平 |
| KEY2 | PE4 | 低电平 |
| KEY3 | PE5 | 低电平 |
| KEY4 | PE6 | 低电平 |
| KEY5 | PE7 | 低电平 |
| KEY6 | PE8 | 低电平 |
| KEY7 | PE9 | 低电平 |
| KEY8 | PE10 | 低电平 |
| KEY9 | PE11 | 低电平 |

CubeMX 需要保持以下配置：

| 项目 | 配置 |
| ---- | ---- |
| PE2~PE11 Mode | `GPIO_EXTI` |
| Trigger | `Rising/Falling edge` |
| Pull-up/Pull-down | `Pull-up` |
| NVIC | 开启 `EXTI2_IRQn`、`EXTI3_IRQn`、`EXTI4_IRQn`、`EXTI9_5_IRQn`、`EXTI15_10_IRQn` |
