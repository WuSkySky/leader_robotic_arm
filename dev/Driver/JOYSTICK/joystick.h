#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

// 摇杆 ADC 通道数量。修改 CubeMX 中 ADC 常规转换通道数量时需同步修改。
#define JOYSTICK_NUM 4

typedef struct {
    int id;
    float value;
    float voltage;
    uint16_t adc_raw;
    uint16_t adc_zero;
    int flag_has_recorded_zero;
} Joystick;

void joystick_init(Joystick* joystick, int id);
void joystick_get_value(Joystick* joystick);
void joystick_record_zero(Joystick* joystick);
HAL_StatusTypeDef joystick_adc_start(void);

#endif
