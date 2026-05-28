#include <main.h>
#include <stdbool.h>
#include "app.h"
#include "board.h"
#include "task_humidity.h"

extern ADC_HandleTypeDef hadc1;

typedef enum {
    TASK_HUMIDITY_ST_WAIT_NEXT_SAMPLE = 0,
    TASK_HUMIDITY_ST_WAIT_ADC_CONVERSION
} task_humidity_state_t;

typedef struct {
    task_humidity_state_t state;
    uint32_t sample_tick_count;
} task_humidity_data_t;

static task_humidity_data_t task_humidity_data;

static bool task_humidity_is_wet(uint16_t adc_value, uint16_t threshold)
{
#if (HUMIDITY_WET_BELOW_THRESHOLD == 1u)
    return (adc_value < threshold);
#else
    return (adc_value > threshold);
#endif
}

void task_humidity_init(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;

    task_humidity_data.state = TASK_HUMIDITY_ST_WAIT_NEXT_SAMPLE;
    task_humidity_data.sample_tick_count = HUMIDITY_SAMPLE_TICKS;

    shared_data->humidity_adc_value = 0u;
    shared_data->humidity_threshold = HUMIDITY_THRESHOLD_DEFAULT;
    shared_data->humidity = false;
    shared_data->humidity_changed = false;
}

void task_humidity_update(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;
    HAL_StatusTypeDef hal_status;
    uint16_t adc_value;
    bool humidity_new;
    ADC_ChannelConfTypeDef sConfig = {0};

    shared_data->humidity_changed = false;

    switch (task_humidity_data.state)
    {
    case TASK_HUMIDITY_ST_WAIT_NEXT_SAMPLE:

        if (task_humidity_data.sample_tick_count < HUMIDITY_SAMPLE_TICKS) {
            task_humidity_data.sample_tick_count++;
            break;
        }

        task_humidity_data.sample_tick_count = 0u;

        if (shared_data->adc_busy == true) {
            break;
        }

        sConfig.Channel = HUMIDITY_ADC_CHANNEL;
        sConfig.Rank = ADC_REGULAR_RANK_1;
        sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

        hal_status = HAL_ADC_ConfigChannel(&hadc1, &sConfig);
        if (hal_status != HAL_OK) {
            break;
        }

        shared_data->adc_busy = true;
        shared_data->adc_owner = ADC_OWNER_HUMIDITY;

        hal_status = HAL_ADC_Start(&hadc1);
        if (hal_status == HAL_OK) {
            task_humidity_data.state = TASK_HUMIDITY_ST_WAIT_ADC_CONVERSION;
        } else {
            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;
        }

        break;

    case TASK_HUMIDITY_ST_WAIT_ADC_CONVERSION:

        if (shared_data->adc_owner != ADC_OWNER_HUMIDITY) {
            task_humidity_data.state = TASK_HUMIDITY_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        hal_status = HAL_ADC_PollForConversion(&hadc1, 0u);

        if (hal_status == HAL_OK) {

            adc_value = (uint16_t) HAL_ADC_GetValue(&hadc1);
            HAL_ADC_Stop(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            shared_data->humidity_adc_value = adc_value;

            humidity_new = task_humidity_is_wet(
                shared_data->humidity_adc_value,
                shared_data->humidity_threshold
            );

            if (humidity_new != shared_data->humidity) {
                shared_data->humidity = humidity_new;
                shared_data->humidity_changed = true;
            }

//            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, shared_data->humidity ? GPIO_PIN_SET : GPIO_PIN_RESET);

            task_humidity_data.state = TASK_HUMIDITY_ST_WAIT_NEXT_SAMPLE;
        }
        else if (hal_status == HAL_ERROR) {
            HAL_ADC_Stop(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            task_humidity_data.state = TASK_HUMIDITY_ST_WAIT_NEXT_SAMPLE;
        }

        break;

    default:
        task_humidity_data.state = TASK_HUMIDITY_ST_WAIT_NEXT_SAMPLE;
        task_humidity_data.sample_tick_count = 0u;
        break;
    }
}
