#include "feetech.h"
#include "SCServo.h"
#include <math.h>
#include "main.h"
#include "usart.h"

static void recode_pos_raw(FEETECHServo* servo)
{
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

static void proc_zero_in_middle_mode(FEETECHServo* servo)
{
    servo->pos_in_zero= servo->pos_raw - servo->pos_zero - 2048;
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
    servo->pos = servo->pos_in_mutil_cycle * 0.087 * (M_PI / 180.0);
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
    proc_zero_in_middle_mode(servo);
    proc_mutil_cycle(servo);
    pos_normalize(servo);
}

//FT舵机串口指令发送函数
void ftUart_Send(uint8_t *nDat , int nLen)
{
	HAL_UART_Transmit(&huart3, nDat, nLen, 100);
}

//FT舵机串口指令应答接收函数
int ftUart_Read(uint8_t *nDat, int nLen)
{
	if(HAL_OK!=HAL_UART_Receive(&huart3, nDat, nLen, 5)){
		return 0;
	}else{
		return nLen;
	}
}

//FT舵机总线切换延时，时间大于10us
void ftBus_Delay(void)
{
	HAL_Delay(1);
}