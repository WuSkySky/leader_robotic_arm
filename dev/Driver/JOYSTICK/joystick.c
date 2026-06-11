#include "joystick.h"
#include "adc.h"

// DMA 会按 ADC rank 1-4 的顺序连续写入该缓冲区。
static uint16_t adc_data[JOYSTICK_NUM];

static int get_adc_index(Joystick* joystick)
{
    int index = joystick->id;

    if (index < 0)
    {
        return 0;
    }

    if (index >= JOYSTICK_NUM)
    {
        return JOYSTICK_NUM - 1;
    }

    return index;
}

static float clamp_value(float value)
{
    if (value > 1.0f)
    {
        return 1.0f;
    }

    if (value < -1.0f)
    {
        return -1.0f;
    }

    return value;
}

static float proc_deadzone(float value)
{
    if ((value > -JOYSTICK_DEADZONE) && (value < JOYSTICK_DEADZONE))
    {
        return 0.0f;
    }

    return value;
}

static void update_adc_raw(Joystick* joystick)
{
    joystick->adc_raw = adc_data[get_adc_index(joystick)];
}

static void proc_voltage(Joystick* joystick)
{
    joystick->voltage = joystick->adc_raw * JOYSTICK_VREF / JOYSTICK_ADC_MAX;
}

static void proc_zero(Joystick* joystick)
{
    float value;

    // 零位到最大值、零位到最小值分开映射，保证校准零位对应 0。
    if (joystick->adc_raw >= joystick->adc_zero)
    {
        float positive_range = JOYSTICK_ADC_MAX - joystick->adc_zero;
        value = positive_range > 0.0f ? (joystick->adc_raw - joystick->adc_zero) / positive_range : 0.0f;
    }
    else
    {
        float negative_range = joystick->adc_zero;
        value = negative_range > 0.0f ? -((joystick->adc_zero - joystick->adc_raw) / negative_range) : 0.0f;
    }

    joystick->value = proc_deadzone(clamp_value(value));
}

/**
 * @brief 启动 ADC1 DMA，连续采集所有摇杆通道。
 * @return HAL_ADC_Start_DMA() 返回的 HAL 状态。
 */
HAL_StatusTypeDef JOYSTICK_adc_start(void)
{
    return HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_data, JOYSTICK_NUM);
}

/**
 * @brief 初始化单个摇杆对象。
 * @param joystick 摇杆对象指针。
 * @param id 摇杆编号，从 0 开始，对应 ADC rank 顺序。
 */
void JOYSTICK_init(Joystick* joystick, int id)
{
    joystick->id = id;
    joystick->value = 0.0f;
    joystick->voltage = 0.0f;
    joystick->adc_raw = 0;
    joystick->adc_zero = 0;
    joystick->flag_has_recorded_zero = 0;
}

/**
 * @brief 将当前 ADC 值记录为摇杆零位。
 * @param joystick 摇杆对象指针。
 */
void JOYSTICK_record_zero(Joystick* joystick)
{
    update_adc_raw(joystick);
    joystick->adc_zero = joystick->adc_raw;
    joystick->flag_has_recorded_zero = 1;
}

/**
 * @brief 兼容旧接口：将当前 ADC 值记录为摇杆零位。
 * @param joystick 摇杆对象指针。
 */
void JOYSTICK_recode_zero(Joystick* joystick)
{
    JOYSTICK_record_zero(joystick);
}

/**
 * @brief 更新单个摇杆的 ADC 原始值、电压和归一化值。
 * @param joystick 摇杆对象指针。
 */
void JOYSTICK_get_value(Joystick* joystick)
{
    update_adc_raw(joystick);

    if (!joystick->flag_has_recorded_zero)
    {
        JOYSTICK_record_zero(joystick);
    }

    proc_voltage(joystick);
    proc_zero(joystick);
}

/**
 * @brief 初始化摇杆数组，并完成 ADC DMA 启动和上电零位校准。
 *
 * 该函数会依次完成摇杆对象初始化、启动 ADC DMA、等待首批采样稳定、记录零位。
 * App 层通常只需要调用该函数，不需要再单独调用 JOYSTICK_adc_start() 或 JOYSTICK_all_record_zero()。
 *
 * @param joysticks 摇杆数组指针。
 * @param num 需要初始化的摇杆数量，应与 JOYSTICK_NUM 保持一致。
 */
void JOYSTICK_all_init(Joystick* joysticks, int num)
{
    for (int i = 0; i < num; i++)
    {
        JOYSTICK_init(&joysticks[i], i);
    }

    JOYSTICK_adc_start();

    // 等待 DMA 缓冲区得到有效 ADC 采样值后再做零位校准。
    HAL_Delay(10);
    JOYSTICK_all_record_zero(joysticks, num);
}

/**
 * @brief 将当前 ADC 值记录为摇杆数组的零位。
 * @param joysticks 摇杆数组指针。
 * @param num 需要校准的摇杆数量。
 */
void JOYSTICK_all_record_zero(Joystick* joysticks, int num)
{
    for (int i = 0; i < num; i++)
    {
        JOYSTICK_record_zero(&joysticks[i]);
    }
}

/**
 * @brief 兼容旧接口：将当前 ADC 值记录为摇杆数组的零位。
 * @param joysticks 摇杆数组指针。
 * @param num 需要校准的摇杆数量。
 */
void JOYSTICK_all_recode_zero(Joystick* joysticks, int num)
{
    JOYSTICK_all_record_zero(joysticks, num);
}

/**
 * @brief 更新摇杆数组的 ADC 原始值、电压和归一化值。
 * @param joysticks 摇杆数组指针。
 * @param num 需要更新的摇杆数量。
 */
void JOYSTICK_all_get_value(Joystick* joysticks, int num)
{
    for (int i = 0; i < num; i++)
    {
        JOYSTICK_get_value(&joysticks[i]);
    }
}
