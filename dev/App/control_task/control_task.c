#include "control_task.h"
#include "key.h"
#include "main.h"
#include "usart.h"
#include <string.h>

// 底盘前后最大速度，单位 m/s。修改底盘协议或底盘限速时，需要同步修改该值。
static const float CHASSIS_VX_MAX = 4.0f;

// 底盘左右平移最大速度，单位 m/s。修改底盘协议或底盘限速时，需要同步修改该值。
static const float CHASSIS_VY_MAX = 4.0f;

// 底盘左右旋转最大角速度，单位 rad/s。修改底盘协议或底盘限速时，需要同步修改该值。
static const float CHASSIS_VW_MAX = 1.57f;

enum
{
    // 串口通讯帧长度。修改 README 中串口帧协议时，需要同步修改该值和 control_send()。
    CONTROL_TX_FRAME_LEN = 29,
    CONTROL_JOYSTICK_NUM = JOYSTICK_NUM
};

// 底盘控制量。
typedef struct
{
    float vx;    // 前后速度，单位 m/s。
    float vy;    // 左右平移速度，单位 m/s。
    float vw;    // 左右旋转角速度，单位 rad/s。
} ChassisControl;

// 控制任务中的机械臂状态变量
static LeaderRoboticArm robotic_arm;

// 控制任务中的摇杆状态变量
static Joystick joysticks[JOYSTICK_NUM];

//keys
static Key keys[KEYS_NUM];

// 控制任务中的底盘控制状态变量
static ChassisControl chassis;

static uint8_t tx_frame_1[CONTROL_TX_FRAME_LEN];
static uint8_t tx_frame_2[CONTROL_TX_FRAME_LEN];

static void joysticks_init(void)
{
    for (int i = 0; i < JOYSTICK_NUM; i++)
    {
        joystick_init(&joysticks[i], i);
    }

    joystick_adc_start();

    // 等待 DMA 缓冲区得到有效 ADC 采样值后再做零位校准。
    HAL_Delay(10);
    for (int i = 0; i < JOYSTICK_NUM; i++)
    {
        joystick_record_zero(&joysticks[i]);
    }
}

static void joysticks_update(void)
{
    for (int i = 0; i < JOYSTICK_NUM; i++)
    {
        joystick_get_value(&joysticks[i]);
    }
}

static void chassis_update(void)
{
    chassis.vx = joysticks[3].value * CHASSIS_VX_MAX;
    chassis.vy = joysticks[2].value * CHASSIS_VY_MAX;
    chassis.vw = joysticks[0].value * CHASSIS_VW_MAX;
}

static void control_send(void)
{
    memset(tx_frame_1, 0, sizeof(tx_frame_1));
    memset(tx_frame_2, 0, sizeof(tx_frame_2));

    tx_frame_1[0] = 0xAA;
    memcpy(tx_frame_1 + 1, robotic_arm.pos, sizeof(robotic_arm.pos));
    tx_frame_1[28] = 0x55;

    tx_frame_2[0] = 0xBB;
    memcpy(tx_frame_2 + 1, &robotic_arm.pos[5], sizeof(robotic_arm.pos[5]));
    memcpy(tx_frame_2 + 5, &chassis.vx, sizeof(chassis.vx));
    memcpy(tx_frame_2 + 9, &chassis.vy, sizeof(chassis.vy));
    memcpy(tx_frame_2 + 13, &chassis.vw, sizeof(chassis.vw));
    tx_frame_2[17] = 0; // 升降台暂未实现，协议字段先固定发送 0。
    tx_frame_2[28] = 0x55;

    HAL_UART_Transmit(&huart1, tx_frame_1, sizeof(tx_frame_1), 10);
    HAL_UART_Transmit(&huart1, tx_frame_2, sizeof(tx_frame_2), 10);
}

static void keys_init(void)
{
    key_init(&keys[0], 0, GPIOE, GPIO_PIN_2, GPIO_PIN_RESET, 20);
    key_init(&keys[1], 1, GPIOE, GPIO_PIN_3, GPIO_PIN_RESET, 20);
    key_init(&keys[2], 2, GPIOE, GPIO_PIN_4, GPIO_PIN_RESET, 20);
    key_init(&keys[3], 3, GPIOE, GPIO_PIN_5, GPIO_PIN_RESET, 20);
    key_init(&keys[4], 4, GPIOE, GPIO_PIN_6, GPIO_PIN_RESET, 20);
    key_init(&keys[5], 5, GPIOE, GPIO_PIN_7, GPIO_PIN_RESET, 20);
    key_init(&keys[6], 6, GPIOE, GPIO_PIN_8, GPIO_PIN_RESET, 20);
    key_init(&keys[7], 7, GPIOE, GPIO_PIN_9, GPIO_PIN_RESET, 20);
    key_init(&keys[8], 8, GPIOE, GPIO_PIN_10, GPIO_PIN_RESET, 20);
    key_init(&keys[9], 9, GPIOE, GPIO_PIN_11, GPIO_PIN_RESET, 20);
}

void control_task_init(void)
{
    // 等待舵机上电启动。
    HAL_Delay(1000);

    leader_robotic_arm_init(&robotic_arm);

    chassis.vx = 0.0f;
    chassis.vy = 0.0f;
    chassis.vw = 0.0f;

    joysticks_init();

    keys_init();
}

/**
 * @brief 更新控制任务数据。
 *
 * 周期读取机械臂位置和摇杆 ADC 数据，计算底盘控制量并通过串口发送。
 */
void control_task_update(void)
{
    leader_robotic_arm_get_pos(&robotic_arm);
    joysticks_update();
    chassis_update();
    control_send();
}
