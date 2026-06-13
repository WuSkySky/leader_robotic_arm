#include "feetech.h"
#include "SCServo.h"
#include <math.h>
#include "main.h"
#include "usart.h"

enum
{
    FEETECH_SERVO_NUM = 6,
    FEETECH_RX_FRAME_LEN = 8,
    FEETECH_SYNC_READ_CMD_LEN = 14,
    FEETECH_ALL_SERVO_MASK = (1 << FEETECH_SERVO_NUM) - 1
};

// 同步读请求超时时间，单位 ms。若舵机总线较慢或回包延迟明显，可适当增大。
static const uint32_t FEETECH_SYNC_READ_TIMEOUT_MS = 5;

// FEETECH/SCS 协议字段，修改舵机型号或寄存器地址时需要同步确认手册。
static const uint8_t FEETECH_BROADCAST_ID = 0xFE;
static const uint8_t FEETECH_INST_SYNC_READ = 0x82;
static const uint8_t FEETECH_PRESENT_POSITION_ADDR = 56;
static const uint8_t FEETECH_PRESENT_POSITION_LEN = 2;

static volatile int pos_raw_cache[FEETECH_SERVO_NUM];
static volatile uint8_t pos_update_seq_cache[FEETECH_SERVO_NUM];
static volatile uint8_t rx_frame[FEETECH_RX_FRAME_LEN];
static volatile uint8_t rx_index;
static uint8_t rx_byte;
static uint8_t rx_started;
static uint8_t sync_read_pending;
static uint8_t sync_read_received_mask;
static uint32_t sync_read_tick;

static int get_servo_index(int id)
{
    int index = id - 1;

    if ((index < 0) || (index >= FEETECH_SERVO_NUM))
    {
        return -1;
    }

    return index;
}

static void uart_rx_start(void)
{
    if (!rx_started)
    {
        rx_started = 1;
        HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
    }
}

static void update_pos_raw_from_cache(FEETECHServo* servo)
{
    int index = get_servo_index(servo->id);
    if (index < 0)
    {
        return;
    }

    if (servo->pos_update_seq == pos_update_seq_cache[index])
    {
        return;
    }

    servo->pos_raw = pos_raw_cache[index];
    servo->pos_update_seq = pos_update_seq_cache[index];
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
    servos->pos_update_seq = 0;
    uart_rx_start();
}

void FEETECH_servo_get_pos(FEETECHServo* servo)
{
    int last_update_seq = servo->pos_update_seq;
    update_pos_raw_from_cache(servo);
    if (last_update_seq == servo->pos_update_seq)
    {
        return;
    }

    proc_zero_in_middle_mode(servo);
    proc_mutil_cycle(servo);
    pos_normalize(servo);
}

void FEETECH_servo_request_all_pos(void)
{
    if (sync_read_pending && ((HAL_GetTick() - sync_read_tick) < FEETECH_SYNC_READ_TIMEOUT_MS))
    {
        return;
    }

    uint8_t tx_buf[FEETECH_SYNC_READ_CMD_LEN];
    uint8_t checksum_sum = 0;

    tx_buf[0] = 0xFF;
    tx_buf[1] = 0xFF;
    tx_buf[2] = FEETECH_BROADCAST_ID;
    tx_buf[3] = FEETECH_SERVO_NUM + 4;
    tx_buf[4] = FEETECH_INST_SYNC_READ;
    tx_buf[5] = FEETECH_PRESENT_POSITION_ADDR;
    tx_buf[6] = FEETECH_PRESENT_POSITION_LEN;

    for (int i = 0; i < FEETECH_SERVO_NUM; i++)
    {
        tx_buf[7 + i] = (uint8_t)(i + 1);
    }

    for (int i = 2; i < (FEETECH_SYNC_READ_CMD_LEN - 1); i++)
    {
        checksum_sum = (uint8_t)(checksum_sum + tx_buf[i]);
    }
    tx_buf[FEETECH_SYNC_READ_CMD_LEN - 1] = (uint8_t)(~checksum_sum);

    sync_read_pending = 1;
    sync_read_received_mask = 0;
    sync_read_tick = HAL_GetTick();
    HAL_UART_Transmit(&huart3, tx_buf, sizeof(tx_buf), 2);
}

static void process_rx_frame(void)
{
    uint8_t id = rx_frame[2];
    uint8_t len = rx_frame[3];
    uint8_t status = rx_frame[4];
    uint8_t pos_l = rx_frame[5];
    uint8_t pos_h = rx_frame[6];
    uint8_t checksum = rx_frame[7];
    uint8_t cal_sum = (uint8_t)(~(id + len + status + pos_l + pos_h));
    int index = get_servo_index(id);

    if ((len != 4) || (checksum != cal_sum) || (index < 0))
    {
        return;
    }

    pos_raw_cache[index] = (int)(pos_l | (pos_h << 8));
    pos_update_seq_cache[index]++;

    if (sync_read_pending)
    {
        sync_read_received_mask |= (uint8_t)(1 << index);
        if (sync_read_received_mask == FEETECH_ALL_SERVO_MASK)
        {
            sync_read_pending = 0;
        }
    }
}

static void uart_rx_parse_byte(uint8_t data)
{
    if (rx_index == 0)
    {
        if (data == 0xFF)
        {
            rx_frame[rx_index++] = data;
        }
        return;
    }

    if (rx_index == 1)
    {
        if (data == 0xFF)
        {
            rx_frame[rx_index++] = data;
        }
        else
        {
            rx_index = 0;
        }
        return;
    }

    rx_frame[rx_index++] = data;
    if (rx_index >= FEETECH_RX_FRAME_LEN)
    {
        process_rx_frame();
        rx_index = 0;
    }
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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        uart_rx_parse_byte(rx_byte);
        HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        rx_index = 0;
        HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
    }
}
