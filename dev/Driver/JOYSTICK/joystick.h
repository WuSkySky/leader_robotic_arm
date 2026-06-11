#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

// 摇杆 ADC 通道数量。修改 CubeMX 中 ADC 常规转换通道数量时，需要同步修改该值。
#define JOYSTICK_NUM 4

// ADC 满量程原始值。12 位 ADC 为 4095；如果 CubeMX 中 ADC 分辨率改为 10 位，应改为 1023。
#define JOYSTICK_ADC_MAX 4095.0f

// ADC 参考电压，单位 V。通常等于 MCU VDDA；如果硬件参考电压不是 3.3V，需要同步修改。
#define JOYSTICK_VREF 3.3f

// 摇杆零位死区。归一化值绝对值小于该值时输出 0；如果零位抖动更大，可以适当增大。
#define JOYSTICK_DEADZONE 0.03f

/**
 * @brief 摇杆通道状态。
 */
typedef struct
{
    // pub
    int id;               // 摇杆编号，从 0 开始。
    float value;          // 归一化后的摇杆值，范围：-1.0f 到 1.0f。
    float voltage;        // ADC 输入电压。

    // priv
    uint16_t adc_raw;     // 最新 ADC 原始值。
    uint16_t adc_zero;    // 零位 ADC 原始值。
    int flag_has_recorded_zero;
} Joystick;

/**
 * @brief 启动 ADC1 DMA，连续采集所有摇杆通道。
 * @return HAL_ADC_Start_DMA() 返回的 HAL 状态。
 */
HAL_StatusTypeDef JOYSTICK_adc_start(void);

/**
 * @brief 初始化单个摇杆对象。
 * @param joystick 摇杆对象指针。
 * @param id 摇杆编号，从 0 开始，对应 ADC rank 顺序。
 */
void JOYSTICK_init(Joystick* joystick, int id);

/**
 * @brief 将当前 ADC 值记录为摇杆零位。
 * @param joystick 摇杆对象指针。
 */
void JOYSTICK_record_zero(Joystick* joystick);

/**
 * @brief 兼容旧接口：将当前 ADC 值记录为摇杆零位。
 * @param joystick 摇杆对象指针。
 */
void JOYSTICK_recode_zero(Joystick* joystick);

/**
 * @brief 更新单个摇杆的 ADC 原始值、电压和归一化值。
 * @param joystick 摇杆对象指针。
 */
void JOYSTICK_get_value(Joystick* joystick);

/**
 * @brief 初始化摇杆数组，并完成 ADC DMA 启动和上电零位校准。
 *
 * 该函数会依次完成摇杆对象初始化、启动 ADC DMA、等待首批采样稳定、记录零位。
 * App 层通常只需要调用该函数，不需要再单独调用 JOYSTICK_adc_start() 或 JOYSTICK_all_record_zero()。
 *
 * @param joysticks 摇杆数组指针。
 * @param num 需要初始化的摇杆数量，应与 JOYSTICK_NUM 保持一致。
 */
void JOYSTICK_all_init(Joystick* joysticks, int num);

/**
 * @brief 将当前 ADC 值记录为摇杆数组的零位。
 * @param joysticks 摇杆数组指针。
 * @param num 需要校准的摇杆数量。
 */
void JOYSTICK_all_record_zero(Joystick* joysticks, int num);

/**
 * @brief 兼容旧接口：将当前 ADC 值记录为摇杆数组的零位。
 * @param joysticks 摇杆数组指针。
 * @param num 需要校准的摇杆数量。
 */
void JOYSTICK_all_recode_zero(Joystick* joysticks, int num);

/**
 * @brief 更新摇杆数组的 ADC 原始值、电压和归一化值。
 * @param joysticks 摇杆数组指针。
 * @param num 需要更新的摇杆数量。
 */
void JOYSTICK_all_get_value(Joystick* joysticks, int num);

#endif
