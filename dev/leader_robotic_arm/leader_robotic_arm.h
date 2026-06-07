#ifndef LEADER_ROBOTIC_ARM_H
#define LEADER_ROBOTIC_ARM_H

#include "feetech.h"

typedef struct
{
    float pos[6];

    FEETECHServo servos[6];
} LeaderRoboticArm;

void leader_robotic_arm_init(LeaderRoboticArm*);

void leader_robotic_arm_get_pos(LeaderRoboticArm*);

#endif