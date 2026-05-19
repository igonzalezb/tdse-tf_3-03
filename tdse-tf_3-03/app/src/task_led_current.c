/*
 * task_led_current.c
 *
 *  Created on: Apr 27, 2026
 *      Author: fmjgo
 */
#include <stdbool.h>
#include "main.h"
#include "app.h"
#include "board.h"
#include "task_led_current.h"
extern ADC_HandleTypeDef hadc1;

typedef enum {
    TASK_LED_CURRENT_ST_WAIT_NEXT_SAMPLE = 0,
    TASK_LED_CURRENT_ST_WAIT_ADC_CONVERSION
} task_led_current_state_t;

typedef struct {
    task_led_current_state_t state;
    uint32_t sample_tick_count;
} task_led_current_data_t;

static task_led_current_data_t task_led_current_data;

static bool task_led_current_is_present(uint16_t adc_value, uint16_t threshold)
{
#if (LED_CURRENT_PRESENT_ABOVE_THRESHOLD == 1u)
    return (adc_value > threshold);
#else
    return (adc_value < threshold);
#endif
}

void task_led_current_init(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;

    task_led_current_data.state = TASK_LED_CURRENT_ST_WAIT_NEXT_SAMPLE;
    task_led_current_data.sample_tick_count = LED_CURRENT_SAMPLE_TICKS;

    shared_data->led_current_adc_value = 0u;
    shared_data->led_current_threshold = LED_CURRENT_THRESHOLD_DEFAULT;
    shared_data->led_current = false;
    shared_data->led_current_changed = false;
}

void task_led_current_update(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;
    HAL_StatusTypeDef hal_status;
    ADC_ChannelConfTypeDef sConfig = {0};
    uint16_t adc_value;
    bool led_current_new;

    shared_data->led_current_changed = false;

    switch (task_led_current_data.state)
    {
    case TASK_LED_CURRENT_ST_WAIT_NEXT_SAMPLE:

        if (task_led_current_data.sample_tick_count < LED_CURRENT_SAMPLE_TICKS) {
            task_led_current_data.sample_tick_count++;
            break;
        }

        task_led_current_data.sample_tick_count = 0u;

        if (shared_data->adc_busy == true) {
            break;
        }

        sConfig.Channel = LED_CURRENT_ADC_CHANNEL;
        sConfig.Rank = ADC_REGULAR_RANK_1;
        sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

        hal_status = HAL_ADC_ConfigChannel(&hadc1, &sConfig);
        if (hal_status != HAL_OK) {
            break;
        }

        shared_data->adc_busy = true;
        shared_data->adc_owner = ADC_OWNER_LED_CURRENT;

        hal_status = HAL_ADC_Start(&hadc1);
        if (hal_status == HAL_OK) {
            task_led_current_data.state = TASK_LED_CURRENT_ST_WAIT_ADC_CONVERSION;
        } else {
            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;
        }

        break;

    case TASK_LED_CURRENT_ST_WAIT_ADC_CONVERSION:

        if (shared_data->adc_owner != ADC_OWNER_LED_CURRENT) {
            task_led_current_data.state = TASK_LED_CURRENT_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        hal_status = HAL_ADC_PollForConversion(&hadc1, 0u);

        if (hal_status == HAL_OK) {

            adc_value = (uint16_t) HAL_ADC_GetValue(&hadc1);
            HAL_ADC_Stop(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            shared_data->led_current_adc_value = adc_value;

            led_current_new = task_led_current_is_present(
                shared_data->led_current_adc_value,
                shared_data->led_current_threshold
            );

            if (led_current_new != shared_data->led_current) {
                shared_data->led_current = led_current_new;
                shared_data->led_current_changed = true;
            }

            task_led_current_data.state = TASK_LED_CURRENT_ST_WAIT_NEXT_SAMPLE;
        }
        else if (hal_status == HAL_ERROR) {
            HAL_ADC_Stop(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            task_led_current_data.state = TASK_LED_CURRENT_ST_WAIT_NEXT_SAMPLE;
        }

        break;

    default:
        task_led_current_data.state = TASK_LED_CURRENT_ST_WAIT_NEXT_SAMPLE;
        task_led_current_data.sample_tick_count = 0u;
        break;
    }
}

