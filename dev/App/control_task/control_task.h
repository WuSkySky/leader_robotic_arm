#ifndef CONTROL_TASK_H
#define CONTROL_TASK_H

#include "joystick.h"
#include "leader_robotic_arm.h"

/**
 * @brief 初始化控制任务。
 *
 * 初始化机械臂、底盘控制量、启动摇杆 ADC DMA，并记录上电后的摇杆零位。
 */
void control_task_init(void);

/**
 * @brief 更新控制任务数据。
 *
 * 周期读取机械臂位置和摇杆 ADC 数据，计算底盘控制量并通过串口发送。
 */
void control_task_update(void);

#endif
