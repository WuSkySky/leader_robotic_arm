#include "leader_robotic_arm.h"
#include "feetech.h"

void leader_robotic_arm_init(LeaderRoboticArm* arm)
{
    for(int i = 0; i < sizeof(arm->servos) / sizeof(arm->servos[0]); i++)
    {
        arm->pos[i] = 0;
        FEETECH_servo_init(&arm->servos[i], i + 1);
    }
}

void leader_robotic_arm_get_pos(LeaderRoboticArm* arm)
{
    for(int i = 0; i < sizeof(arm->servos) / sizeof(arm->servos[0]); i++)
    {
        FEETECH_servo_get_pos(&arm->servos[i]);
        arm->pos[i] = arm->servos[i].pos;
    }

    FEETECH_servo_request_all_pos();
}
