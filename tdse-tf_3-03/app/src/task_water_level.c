#include <stdbool.h>
#include "main.h"
#include "app.h"
#include "board.h"
#include "task_water_level.h"

extern ADC_HandleTypeDef hadc1;

typedef enum {
    TASK_WATER_LEVEL_ST_WAIT_NEXT_SAMPLE = 0,
    TASK_WATER_LEVEL_ST_WAIT_ADC_CONVERSION
} task_water_level_state_t;

typedef struct {
    task_water_level_state_t state;
    uint32_t sample_tick_count;
} task_water_level_data_t;

static task_water_level_data_t task_water_level_data;

static bool task_water_level_is_present(uint16_t adc_value, uint16_t threshold)
{
#if (WATER_LEVEL_PRESENT_ABOVE_THRESHOLD == 1u)
    return (adc_value > threshold);
#else
    return (adc_value < threshold);
#endif
}

void task_water_level_init(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;

    task_water_level_data.state = TASK_WATER_LEVEL_ST_WAIT_NEXT_SAMPLE;
    task_water_level_data.sample_tick_count = WATER_LEVEL_SAMPLE_TICKS;

    shared_data->water_level_adc_value = 0u;
    shared_data->water_level_threshold = WATER_LEVEL_THRESHOLD_DEFAULT;
    shared_data->water_level = false;
    shared_data->water_level_changed = false;
}

void task_water_level_update(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;
    HAL_StatusTypeDef hal_status;
    ADC_ChannelConfTypeDef sConfig = {0};
    uint16_t adc_value;
    bool water_level_new;

    shared_data->water_level_changed = false;

    switch (task_water_level_data.state)
    {
    case TASK_WATER_LEVEL_ST_WAIT_NEXT_SAMPLE:

        if (task_water_level_data.sample_tick_count < WATER_LEVEL_SAMPLE_TICKS) {
            task_water_level_data.sample_tick_count++;
            break;
        }

        task_water_level_data.sample_tick_count = 0u;

        if (shared_data->adc_busy == true) {
            break;
        }

        sConfig.Channel = WATER_LEVEL_ADC_CHANNEL;
        sConfig.Rank = ADC_REGULAR_RANK_1;
        sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

        hal_status = HAL_ADC_ConfigChannel(&hadc1, &sConfig);
        if (hal_status != HAL_OK) {
            break;
        }

        shared_data->adc_busy = true;
        shared_data->adc_owner = ADC_OWNER_WATER_LEVEL;

        hal_status = HAL_ADC_Start(&hadc1);
        if (hal_status == HAL_OK) {
            task_water_level_data.state = TASK_WATER_LEVEL_ST_WAIT_ADC_CONVERSION;
        } else {
            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;
        }

        break;

    case TASK_WATER_LEVEL_ST_WAIT_ADC_CONVERSION:

        if (shared_data->adc_owner != ADC_OWNER_WATER_LEVEL) {
            task_water_level_data.state = TASK_WATER_LEVEL_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        hal_status = HAL_ADC_PollForConversion(&hadc1, 0u);

        if (hal_status == HAL_OK) {

            adc_value = (uint16_t) HAL_ADC_GetValue(&hadc1);
            HAL_ADC_Stop(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            shared_data->water_level_adc_value = adc_value;

            water_level_new = task_water_level_is_present(
                shared_data->water_level_adc_value,
                shared_data->water_level_threshold
            );

            if (water_level_new != shared_data->water_level) {
                shared_data->water_level = water_level_new;
                shared_data->water_level_changed = true;
            }

            task_water_level_data.state = TASK_WATER_LEVEL_ST_WAIT_NEXT_SAMPLE;
        }
        else if (hal_status == HAL_ERROR) {
            HAL_ADC_Stop(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            task_water_level_data.state = TASK_WATER_LEVEL_ST_WAIT_NEXT_SAMPLE;
        }

        break;

    default:
        task_water_level_data.state = TASK_WATER_LEVEL_ST_WAIT_NEXT_SAMPLE;
        task_water_level_data.sample_tick_count = 0u;
        break;
    }
}
