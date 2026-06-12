#ifndef KEY_H
#define KEY_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

enum
{
    // 按键数量。修改 PE2~PE11 按键数量时，需要同步修改该值和 key.c 中的引脚表。
    KEY_NUM = 10
};

/**
 * @brief 初始化按键模块。
 *
 * 读取当前 GPIO 电平作为初始稳定状态，并清空按键事件。
 */
void KEY_init(void);

/**
 * @brief 更新按键事件。
 *
 * 将中断中捕获到的按下/松开事件同步到主循环可读事件。
 * KEY_was_pressed() 和 KEY_was_released() 的结果只在本次 update 后有效。
 */
void KEY_update(void);

/**
 * @brief 获取按键当前稳定状态。
 * @param id 按键编号，从 0 开始，KEY0 对应 PE2。
 * @return 非 0 表示按键处于按下状态，0 表示松开。
 */
uint8_t KEY_is_pressed(int id);

/**
 * @brief 获取按键本轮是否产生按下事件。
 * @param id 按键编号，从 0 开始，KEY0 对应 PE2。
 * @return 非 0 表示本轮产生按下事件，0 表示没有。
 */
uint8_t KEY_was_pressed(int id);

/**
 * @brief 获取按键本轮是否产生松开事件。
 * @param id 按键编号，从 0 开始，KEY0 对应 PE2。
 * @return 非 0 表示本轮产生松开事件，0 表示没有。
 */
uint8_t KEY_was_released(int id);

/**
 * @brief GPIO EXTI 回调入口。
 * @param GPIO_Pin HAL 回调传入的 GPIO pin。
 *
 * 在 HAL_GPIO_EXTI_Callback() 中调用该函数。
 */
void KEY_exti_callback(uint16_t GPIO_Pin);

#endif
