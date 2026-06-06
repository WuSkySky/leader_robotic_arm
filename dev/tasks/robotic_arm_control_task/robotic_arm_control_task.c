#include "robotic_arm_control_task.h"
#include "leader_robotic_arm.h"
#include "usart.h"
#include <string.h>

static LeaderRoboticArm robotic_arm;

static void robotic_arm_send()
{
    uint8_t tx_buf[32];

    tx_buf[0] = 0x00; // dst_addr_h
    tx_buf[1] = 0x02; // dst_addr_l
    tx_buf[2] = 0x01; // channel
    tx_buf[3] = 0xAA; // head
    memcpy(tx_buf + 4, robotic_arm.pos, sizeof(robotic_arm.pos)); // pos
    tx_buf[31] = 0x55; // tail

    // 发送
    HAL_UART_Transmit
    (
        &huart1,
        tx_buf,
        sizeof(tx_buf),
        10
    );
    // uint8_t test_buf[4] = {0x00, 0x02, 0x01, 0xAA};

    // HAL_UART_Transmit(&huart1, test_buf, sizeof(test_buf),10);
}

void robotic_arm_control_task_init(void)
{
    //等待舵机上电启动
    HAL_Delay(1000);

    leader_robotic_arm_init(&robotic_arm);

    HAL_Delay(1000);
}

void robotic_arm_control_task_update(void)
{
    leader_robotic_arm_get_pos(&robotic_arm);

    robotic_arm_send();
}