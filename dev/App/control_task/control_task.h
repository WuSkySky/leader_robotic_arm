#ifndef CONTROL_TASK_H
#define CONTROL_TASK_H

#include "joystick.h"
#include "leader_robotic_arm.h"

// 控制任务管理的摇杆数量。修改摇杆硬件数量或 ADC 通道数量时，需要同步检查该值。
#define CONTROL_TASK_JOYSTICK_NUM JOYSTICK_NUM

// 底盘前后最大速度，单位 m/s。修改底盘协议或底盘限速时，需要同步修改该值。
#define CHASSIS_VX_MAX 4.0f

// 底盘左右平移最大速度，单位 m/s。修改底盘协议或底盘限速时，需要同步修改该值。
#define CHASSIS_VY_MAX 4.0f

// 底盘左右旋转最大角速度，单位 rad/s。修改底盘协议或底盘限速时，需要同步修改该值。
#define CHASSIS_VW_MAX 1.57f

/**
 * @brief 底盘控制量。
 */
typedef struct
{
    float vx;    // 前后速度，单位 m/s。
    float vy;    // 左右平移速度，单位 m/s。
    float vw;    // 左右旋转角速度，单位 rad/s。
} ChassisControl;

// 控制任务中的机械臂状态变量
extern LeaderRoboticArm robotic_arm;

// 控制任务中的摇杆状态变量
extern Joystick joysticks[CONTROL_TASK_JOYSTICK_NUM];

// 控制任务中的底盘控制状态变量
extern ChassisControl chassis;

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
