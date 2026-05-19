#include "main.h"
#include "app.h"
#include "board.h"
#include "task_light.h"
#include <stdbool.h>

extern ADC_HandleTypeDef hadc1;

typedef enum {
    TASK_LIGHT_ST_WAIT_NEXT_SAMPLE = 0,
    TASK_LIGHT_ST_WAIT_ADC_CONVERSION
} task_light_state_t;

typedef struct {
    task_light_state_t state;
    uint32_t sample_tick_count;
} task_light_data_t;

static task_light_data_t task_light_data;

static bool task_light_is_present(uint16_t adc_value, uint16_t threshold)
{
#if (LIGHT_PRESENT_ABOVE_THRESHOLD == 1u)
    return (adc_value > threshold);
#else
    return (adc_value < threshold);
#endif
}

void task_light_init(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;

    task_light_data.state = TASK_LIGHT_ST_WAIT_NEXT_SAMPLE;
    task_light_data.sample_tick_count = LIGHT_SAMPLE_TICKS;

    shared_data->light_adc_value = 0u;
    shared_data->light_threshold = LIGHT_THRESHOLD_DEFAULT;
    shared_data->light = false;
    shared_data->light_changed = false;
}

void task_light_update(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;
    HAL_StatusTypeDef hal_status;
    ADC_ChannelConfTypeDef sConfig = {0};
    uint16_t adc_value;
    bool light_new;

    shared_data->light_changed = false;

    switch (task_light_data.state)
    {
    case TASK_LIGHT_ST_WAIT_NEXT_SAMPLE:

        if (task_light_data.sample_tick_count < LIGHT_SAMPLE_TICKS) {
            task_light_data.sample_tick_count++;
            break;
        }

        task_light_data.sample_tick_count = 0u;

        if (shared_data->adc_busy == true) {
            break;
        }

        sConfig.Channel = LIGHT_ADC_CHANNEL;
        sConfig.Rank = ADC_REGULAR_RANK_1;
        sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

        hal_status = HAL_ADC_ConfigChannel(&hadc1, &sConfig);
        if (hal_status != HAL_OK) {
            break;
        }

        shared_data->adc_busy = true;
        shared_data->adc_owner = ADC_OWNER_LIGHT;

        hal_status = HAL_ADC_Start(&hadc1);
        if (hal_status == HAL_OK) {
            task_light_data.state = TASK_LIGHT_ST_WAIT_ADC_CONVERSION;
        } else {
            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;
        }

        break;

    case TASK_LIGHT_ST_WAIT_ADC_CONVERSION:

        if (shared_data->adc_owner != ADC_OWNER_LIGHT) {
            task_light_data.state = TASK_LIGHT_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        hal_status = HAL_ADC_PollForConversion(&hadc1, 0u);

        if (hal_status == HAL_OK) {

            adc_value = (uint16_t) HAL_ADC_GetValue(&hadc1);
            HAL_ADC_Stop(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            shared_data->light_adc_value = adc_value;

            light_new = task_light_is_present(
                shared_data->light_adc_value,
                shared_data->light_threshold
            );

            if (light_new != shared_data->light) {
                shared_data->light = light_new;
                shared_data->light_changed = true;
            }

            task_light_data.state = TASK_LIGHT_ST_WAIT_NEXT_SAMPLE;
        }
        else if (hal_status == HAL_ERROR) {
            HAL_ADC_Stop(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            task_light_data.state = TASK_LIGHT_ST_WAIT_NEXT_SAMPLE;
        }

        break;

    default:
        task_light_data.state = TASK_LIGHT_ST_WAIT_NEXT_SAMPLE;
        task_light_data.sample_tick_count = 0u;
        break;
    }
}
