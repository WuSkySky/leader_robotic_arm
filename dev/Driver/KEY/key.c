#include "key.h"

static Key* keys[KEYS_NUM] = {0};

void key_init(Key* key, int id, GPIO_TypeDef* port, uint16_t pin, GPIO_PinState pressed_level, uint32_t debounce_ms)
{
    key->id = id;
    key->port = port;
    key->pin = pin;
    key->pressed_level = pressed_level;
    key->debounce_ms = debounce_ms;
    key->is_pressed = 0;

    keys[id] = key;
}

static void key_irq_handle(Key* key)
{
    GPIO_PinState pin_state = HAL_GPIO_ReadPin(key->port, key->pin);
    if (pin_state == key->pressed_level) {
        key->is_pressed = 1;
    } else {
        key->is_pressed = 0;
    }
}   

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    for (int i = 0; i < KEYS_NUM; i++) {
        if (keys[i] != NULL && keys[i]->pin == GPIO_Pin) {
            key_irq_handle(keys[i]);
            break;
        }
    }
}
