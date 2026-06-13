#include "joystick.h"
#include "adc.h"

// ADC 满量程原始值。12 位 ADC 为 4095；若 CubeMX 中改为 10 位分辨率则改为 1023。
static const float ADC_MAX = 4095.0f;

// ADC 参考电压，单位 V。通常等于 MCU VDDA；硬件参考电压改变时需同步修改。
static const float VREF = 3.3f;

// 摇杆零位死区。归一化值绝对值小于该值时输出 0；零位抖动量偏大可适当增大。
static const float DEADZONE = 0.03f;

// DMA 按 ADC rank 顺序连续写入该缓冲区，各通道数据由 id 索引读取。
static uint16_t adc_data[JOYSTICK_NUM];

static int get_adc_index(Joystick* joystick)
{
    int index = joystick->id;

    if (index < 0) return 0;
    if (index >= JOYSTICK_NUM) return JOYSTICK_NUM - 1;

    return index;
}

static float clamp_value(float value)
{
    if (value > 1.0f) return 1.0f;
    if (value < -1.0f) return -1.0f;
    return value;
}

static float proc_deadzone(float value)
{
    if ((value > -DEADZONE) && (value < DEADZONE)) return 0.0f;
    return value;
}

static void update_adc_raw(Joystick* joystick)
{
    joystick->adc_raw = adc_data[get_adc_index(joystick)];
}

static void proc_voltage(Joystick* joystick)
{
    joystick->voltage = joystick->adc_raw * VREF / ADC_MAX;
}

static void proc_zero(Joystick* joystick)
{
    float value;

    // 零位到正最大值、零位到负最大值分开映射，保证校准零位映射到 0。
    if (joystick->adc_raw >= joystick->adc_zero)
    {
        float positive_range = ADC_MAX - joystick->adc_zero;
        value = positive_range > 0.0f ? (joystick->adc_raw - joystick->adc_zero) / positive_range : 0.0f;
    }
    else
    {
        float negative_range = joystick->adc_zero;
        value = negative_range > 0.0f ? -((joystick->adc_zero - joystick->adc_raw) / negative_range) : 0.0f;
    }

    joystick->value = proc_deadzone(clamp_value(value));
}

HAL_StatusTypeDef joystick_adc_start(void)
{
    return HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_data, JOYSTICK_NUM);
}

void joystick_init(Joystick* joystick, int id)
{
    joystick->id = id;
    joystick->value = 0.0f;
    joystick->voltage = 0.0f;
    joystick->adc_raw = 0;
    joystick->adc_zero = 0;
    joystick->flag_has_recorded_zero = 0;
}

void joystick_record_zero(Joystick* joystick)
{
    update_adc_raw(joystick);
    joystick->adc_zero = joystick->adc_raw;
    joystick->flag_has_recorded_zero = 1;
}

void joystick_get_value(Joystick* joystick)
{
    update_adc_raw(joystick);

    if (!joystick->flag_has_recorded_zero)
    {
        joystick_record_zero(joystick);
    }

    proc_voltage(joystick);
    proc_zero(joystick);
}
