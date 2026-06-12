#ifndef KEY_H
#define KEY_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

#define KEYS_NUM 9

typedef struct{
    int id;
    GPIO_TypeDef* port;
    uint16_t pin; 
    GPIO_PinState pressed_level;
    uint32_t debounce_ms; 
    uint8_t is_pressed;
} Key;

void key_init(Key* key, int id, GPIO_TypeDef* port, uint16_t pin, GPIO_PinState pressed_level, uint32_t debounce_ms);

#endif
