/*
 * task_pump_current.c
 *
 *  Created on: Apr 27, 2026
 *      Author: fmjgo
 */
#include "main.h"
#include "app.h"
#include "board.h"
#include "task_pump_current.h"

extern ADC_HandleTypeDef hadc1;

typedef enum {
    TASK_PUMP_CURRENT_ST_WAIT_NEXT_SAMPLE = 0,
    TASK_PUMP_CURRENT_ST_WAIT_ADC_CONVERSION
} task_pump_current_state_t;

typedef struct {
    task_pump_current_state_t state;
    uint32_t sample_tick_count;
} task_pump_current_data_t;

static task_pump_current_data_t task_pump_current_data;

static bool task_pump_current_is_present(uint16_t adc_value, uint16_t threshold)
{
#if (PUMP_CURRENT_PRESENT_ABOVE_THRESHOLD == 1u)
    return (adc_value > threshold);
#else
    return (adc_value < threshold);
#endif
}

void task_pump_current_init(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;

    task_pump_current_data.state = TASK_PUMP_CURRENT_ST_WAIT_NEXT_SAMPLE;
    task_pump_current_data.sample_tick_count = PUMP_CURRENT_SAMPLE_TICKS;

    shared_data->pump_current_adc_value = 0u;
    shared_data->pump_current_threshold = PUMP_CURRENT_THRESHOLD_DEFAULT;
    shared_data->pump_current = false;
    shared_data->pump_current_changed = false;
}

void task_pump_current_update(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;
    HAL_StatusTypeDef hal_status;
    ADC_ChannelConfTypeDef sConfig = {0};
    uint16_t adc_value;
    bool pump_current_new;

    shared_data->pump_current_changed = false;

    switch (task_pump_current_data.state)
    {
    case TASK_PUMP_CURRENT_ST_WAIT_NEXT_SAMPLE:

        if (task_pump_current_data.sample_tick_count < PUMP_CURRENT_SAMPLE_TICKS) {
            task_pump_current_data.sample_tick_count++;
            break;
        }

        task_pump_current_data.sample_tick_count = 0u;

        if (shared_data->adc_busy == true) {
            break;
        }

        sConfig.Channel = PUMP_CURRENT_ADC_CHANNEL;
        sConfig.Rank = ADC_REGULAR_RANK_1;
        sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

        hal_status = HAL_ADC_ConfigChannel(&hadc1, &sConfig);
        if (hal_status != HAL_OK) {
            break;
        }

        shared_data->adc_busy = true;
        shared_data->adc_owner = ADC_OWNER_PUMP_CURRENT;

        hal_status = HAL_ADC_Start(&hadc1);
        if (hal_status == HAL_OK) {
            task_pump_current_data.state = TASK_PUMP_CURRENT_ST_WAIT_ADC_CONVERSION;
        } else {
            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;
        }

        break;

    case TASK_PUMP_CURRENT_ST_WAIT_ADC_CONVERSION:

        if (shared_data->adc_owner != ADC_OWNER_PUMP_CURRENT) {
            task_pump_current_data.state = TASK_PUMP_CURRENT_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        hal_status = HAL_ADC_PollForConversion(&hadc1, 0u);

        if (hal_status == HAL_OK) {

            adc_value = (uint16_t) HAL_ADC_GetValue(&hadc1);
            HAL_ADC_Stop(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            shared_data->pump_current_adc_value = adc_value;

            pump_current_new = task_pump_current_is_present(
                shared_data->pump_current_adc_value,
                shared_data->pump_current_threshold
            );

            if (pump_current_new != shared_data->pump_current) {
                shared_data->pump_current = pump_current_new;
                shared_data->pump_current_changed = true;
            }

            task_pump_current_data.state = TASK_PUMP_CURRENT_ST_WAIT_NEXT_SAMPLE;
        }
        else if (hal_status == HAL_ERROR) {
            HAL_ADC_Stop(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            task_pump_current_data.state = TASK_PUMP_CURRENT_ST_WAIT_NEXT_SAMPLE;
        }

        break;

    default:
        task_pump_current_data.state = TASK_PUMP_CURRENT_ST_WAIT_NEXT_SAMPLE;
        task_pump_current_data.sample_tick_count = 0u;
        break;
    }
}

