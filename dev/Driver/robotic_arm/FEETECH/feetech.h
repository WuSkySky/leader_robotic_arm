#ifndef FEETECH_H
#define FEETECH_H

typedef struct
{
    /** 舵机总线 ID，当前机械臂使用 1~6。 */
    int id;

    /** 舵机当前位置，单位 rad。 */
    float pos;

    int pos_raw;

    int pos_zero;
    int flag_has_recoded_zero;
    int pos_in_zero;
    int pos_in_zero_last;

    int pos_in_mutil_cycle;
    int cycle;
    int pos_update_seq;
} FEETECHServo;

/**
 * @brief 初始化一个 FEETECH 舵机对象。
 *
 * @param servo 舵机对象指针。
 * @param id 舵机总线 ID，当前机械臂使用 1~6。
 */
void FEETECH_servo_init(FEETECHServo*, int);

/**
 * @brief 从接收缓存更新舵机位置。
 *
 * 该函数不会阻塞等待串口回包，只会在中断缓存收到新位置后更新 `servo->pos`。
 *
 * @param servo 舵机对象指针。
 */
void FEETECH_servo_get_pos(FEETECHServo*);

/**
 * @brief 向 1~6 号舵机发送同步读位置请求。
 *
 * 回包由 USART3 接收中断解析。调用频率应与主循环一致，当前控制任务为 60Hz。
 */
void FEETECH_servo_request_all_pos(void);

#endif
