#include "get_robotic_arm_pos_feedback.h"
#include "SCServo.h"
#include "main.h"
#include <sys/errno.h>

int robotic_arm_pos_feedback[7] = {0};

void setup_get_robotic_arm_pos_feedback(void)
{
    setEnd(0);
}

void task_get_robotic_arm_pos_feedback(void)
{
    for(int i=0; i<=5; i++)
    {
        if(!getLastError())
        {
            robotic_arm_pos_feedback[i] = ReadPos(i+1);
        }
    }
}