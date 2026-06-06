#include "feetech.h"
#include "SCServo.h"

static void recode_pos_raw(FEETECHServo* servo)
{
    if(getLastError()) return;

    servo->pos_raw = ReadPos(servo->id);
}

static void recode_zero(FEETECHServo* servo)
{
    servo->pos_zero = servo->pos_raw;
    servo->flag_has_recoded_zero = 1;
    
}

static void proc_zero(FEETECHServo* servo)
{
    servo->pos_in_zero= servo->pos_raw - servo->pos_zero;
}

static void proc_mutil_cycle(FEETECHServo* servo)
{
    if (servo->pos_in_zero - servo->pos_in_zero_last > 2048)
    {
        servo->cycle--;
    }
    else if (servo->pos_in_zero - servo->pos_in_zero_last < -2048)
    {
         servo->cycle++;
    }
    servo->pos_in_mutil_cycle = servo->pos_in_zero + servo->cycle * 4096;

    servo->pos_in_zero_last = servo->pos_in_zero;
}

static void pos_normalize(FEETECHServo* servo)
{
    servo->pos = servo->pos_in_mutil_cycle*0.087;
}

void FEETECH_servo_init(FEETECHServo* servos, int id)
{
    servos->id = id;
    servos->pos_raw = 0;
    servos->pos_zero = 0;
    servos->flag_has_recoded_zero = 0;
    servos->pos_in_zero = 0;
    servos->pos_in_zero_last = 0;
    servos->pos_in_mutil_cycle = 0;
    servos->cycle = 0;
}

void FEETECH_servo_get_pos(FEETECHServo* servo)
{
    recode_pos_raw(servo);

    // if (!servo->flag_has_recoded_zero) 
    // {
    //     recode_zero(servo);
    // }
    proc_zero(servo);
    proc_mutil_cycle(servo);
    pos_normalize(servo);
}