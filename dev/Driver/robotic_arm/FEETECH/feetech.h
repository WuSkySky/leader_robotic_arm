#ifndef FEETECH_H
#define FEETECH_H

typedef struct
{
    // pub
    int id;
    float pos;

    // priv
    int pos_raw;

    int pos_zero;
    int flag_has_recoded_zero;
    int pos_in_zero;
    int pos_in_zero_last;

    int pos_in_mutil_cycle;
    int cycle;
} FEETECHServo;

void FEETECH_servo_init(FEETECHServo*, int);

void FEETECH_servo_get_pos(FEETECHServo*);

#endif