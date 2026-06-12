#include "key.h"

// 按键消抖时间，单位 ms。若按键抖动仍明显可调大，若响应偏慢可调小。
static const uint32_t KEY_DEBOUNCE_MS = 30;

// 按键按下接 GND，因此 GPIO 低电平表示按下。
static const GPIO_PinState KEY_PRESSED_LEVEL = GPIO_PIN_RESET;

static GPIO_TypeDef* const key_ports[KEY_NUM] = {
    GPIOE, GPIOE, GPIOE, GPIOE, GPIOE,
    GPIOE, GPIOE, GPIOE, GPIOE, GPIOE
};

static const uint16_t key_pins[KEY_NUM] = {
    GPIO_PIN_2,
    GPIO_PIN_3,
    GPIO_PIN_4,
    GPIO_PIN_5,
    GPIO_PIN_6,
    GPIO_PIN_7,
    GPIO_PIN_8,
    GPIO_PIN_9,
    GPIO_PIN_10,
    GPIO_PIN_11
};

static volatile uint8_t key_stable_pressed[KEY_NUM];
static volatile uint8_t key_pending_pressed[KEY_NUM];
static volatile uint8_t key_pending_released[KEY_NUM];
static volatile uint32_t key_last_irq_tick[KEY_NUM];

static uint8_t key_current_pressed_event[KEY_NUM];
static uint8_t key_current_released_event[KEY_NUM];

static int key_get_id_from_pin(uint16_t GPIO_Pin)
{
    for (int i = 0; i < KEY_NUM; i++)
    {
        if (key_pins[i] == GPIO_Pin)
        {
            return i;
        }
    }

    return -1;
}

static uint8_t key_read_pressed(int id)
{
    return HAL_GPIO_ReadPin(key_ports[id], key_pins[id]) == KEY_PRESSED_LEVEL;
}

/**
 * @brief 初始化按键模块。
 *
 * 读取当前 GPIO 电平作为初始稳定状态，并清空按键事件。
 */
void KEY_init(void)
{
    uint32_t now = HAL_GetTick();

    for (int i = 0; i < KEY_NUM; i++)
    {
        key_stable_pressed[i] = key_read_pressed(i);
        key_pending_pressed[i] = 0;
        key_pending_released[i] = 0;
        key_current_pressed_event[i] = 0;
        key_current_released_event[i] = 0;
        key_last_irq_tick[i] = now - KEY_DEBOUNCE_MS;
    }
}

/**
 * @brief 更新按键事件。
 *
 * 将中断中捕获到的按下/松开事件同步到主循环可读事件。
 * KEY_was_pressed() 和 KEY_was_released() 的结果只在本次 update 后有效。
 */
void KEY_update(void)
{
    __disable_irq();
    for (int i = 0; i < KEY_NUM; i++)
    {
        key_current_pressed_event[i] = key_pending_pressed[i];
        key_current_released_event[i] = key_pending_released[i];
        key_pending_pressed[i] = 0;
        key_pending_released[i] = 0;
    }
    __enable_irq();
}

/**
 * @brief 获取按键当前稳定状态。
 * @param id 按键编号，从 0 开始，KEY0 对应 PE2。
 * @return 非 0 表示按键处于按下状态，0 表示松开。
 */
uint8_t KEY_is_pressed(int id)
{
    if ((id < 0) || (id >= KEY_NUM))
    {
        return 0;
    }

    return key_stable_pressed[id];
}

/**
 * @brief 获取按键本轮是否产生按下事件。
 * @param id 按键编号，从 0 开始，KEY0 对应 PE2。
 * @return 非 0 表示本轮产生按下事件，0 表示没有。
 */
uint8_t KEY_was_pressed(int id)
{
    if ((id < 0) || (id >= KEY_NUM))
    {
        return 0;
    }

    return key_current_pressed_event[id];
}

/**
 * @brief 获取按键本轮是否产生松开事件。
 * @param id 按键编号，从 0 开始，KEY0 对应 PE2。
 * @return 非 0 表示本轮产生松开事件，0 表示没有。
 */
uint8_t KEY_was_released(int id)
{
    if ((id < 0) || (id >= KEY_NUM))
    {
        return 0;
    }

    return key_current_released_event[id];
}

/**
 * @brief GPIO EXTI 回调入口。
 * @param GPIO_Pin HAL 回调传入的 GPIO pin。
 *
 * 在 HAL_GPIO_EXTI_Callback() 中调用该函数。
 */
void KEY_exti_callback(uint16_t GPIO_Pin)
{
    int id = key_get_id_from_pin(GPIO_Pin);
    if (id < 0)
    {
        return;
    }

    uint32_t now = HAL_GetTick();
    if ((now - key_last_irq_tick[id]) < KEY_DEBOUNCE_MS)
    {
        return;
    }
    key_last_irq_tick[id] = now;

    uint8_t pressed = key_read_pressed(id);
    if (pressed == key_stable_pressed[id])
    {
        return;
    }

    key_stable_pressed[id] = pressed;
    if (pressed)
    {
        key_pending_pressed[id] = 1;
    }
    else
    {
        key_pending_released[id] = 1;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    KEY_exti_callback(GPIO_Pin);
}
