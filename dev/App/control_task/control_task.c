#include "control_task.h"
#include "main.h"
#include "usart.h"
#include <string.h>

// 串口控制帧长度：帧头 1 字节 + 机械臂 6 个 float + 底盘 3 个 float + 帧尾 1 字节。
#define CONTROL_TX_FRAME_LEN 38

LeaderRoboticArm robotic_arm;
Joystick joysticks[CONTROL_TASK_JOYSTICK_NUM];
ChassisControl chassis;

static uint8_t tx_buf[CONTROL_TX_FRAME_LEN];

static void chassis_update(void)
{
    chassis.vx = joysticks[3].value * CHASSIS_VX_MAX;
    chassis.vy = joysticks[2].value * CHASSIS_VY_MAX;
    chassis.vw = joysticks[0].value * CHASSIS_VW_MAX;
}

static void control_send(void)
{
    tx_buf[0] = 0xAA;
    memcpy(tx_buf + 1, robotic_arm.pos, sizeof(robotic_arm.pos));
    memcpy(tx_buf + 25, &chassis.vx, sizeof(chassis.vx));
    memcpy(tx_buf + 29, &chassis.vy, sizeof(chassis.vy));
    memcpy(tx_buf + 33, &chassis.vw, sizeof(chassis.vw));
    tx_buf[37] = 0x55;

    HAL_UART_Transmit(&huart1, tx_buf, sizeof(tx_buf), 10);
}

/**
 * @brief 初始化控制任务。
 *
 * 初始化机械臂、底盘控制量、启动摇杆 ADC DMA，并记录上电后的摇杆零位。
 */
void control_task_init(void)
{
    // 等待舵机上电启动。
    HAL_Delay(1000);

    leader_robotic_arm_init(&robotic_arm);

    chassis.vx = 0.0f;
    chassis.vy = 0.0f;
    chassis.vw = 0.0f;

    JOYSTICK_all_init(joysticks, CONTROL_TASK_JOYSTICK_NUM);
}

/**
 * @brief 更新控制任务数据。
 *
 * 周期读取机械臂位置和摇杆 ADC 数据，计算底盘控制量并通过串口发送。
 */
void control_task_update(void)
{
    leader_robotic_arm_get_pos(&robotic_arm);
    JOYSTICK_all_get_value(joysticks, CONTROL_TASK_JOYSTICK_NUM);
    chassis_update();
    control_send();
}
