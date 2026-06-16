/*
 * task_pump_current.c
 *
 *  Created on: Apr 27, 2026
 *      Author: fmjgo
 */
#include <stdbool.h>
#include "main.h"
#include "app.h"
#include "board.h"
#include "task_pump_current.h"

extern ADC_HandleTypeDef hadc1;

/*
 * Constantes de calibracion para convertir la lectura ADC a porcentaje.
 * Ajustar estos valores midiendo en Live Expressions:
 * - PUMP_CURRENT_ADC_NO_CURRENT: bomba apagada / sin corriente
 * - PUMP_CURRENT_ADC_MAX_CURRENT: bomba encendida en condicion normal/maxima
 */
#define PUMP_CURRENT_ADC_NO_CURRENT     0u
#define PUMP_CURRENT_ADC_MAX_CURRENT    4095u

typedef enum {
    TASK_PUMP_CURRENT_ST_WAIT_NEXT_SAMPLE = 0,
    TASK_PUMP_CURRENT_ST_WAIT_ADC_CONVERSION
} task_pump_current_state_t;

typedef struct {
    task_pump_current_state_t state;
    uint32_t sample_tick_count;
} task_pump_current_data_t;

static task_pump_current_data_t task_pump_current_data;

static uint8_t task_pump_current_adc_to_percent(uint16_t adc_value)
{
    int32_t percent;
    int32_t adc_no_current = (int32_t)PUMP_CURRENT_ADC_NO_CURRENT;
    int32_t adc_max_current = (int32_t)PUMP_CURRENT_ADC_MAX_CURRENT;
    int32_t adc = (int32_t)adc_value;

    if (adc_no_current == adc_max_current) {
        return 0u;
    }

    /* Caso normal: el ADC aumenta cuando aumenta la corriente */
    if (adc_max_current > adc_no_current) {
        if (adc <= adc_no_current) {
            return 0u;
        }

        if (adc >= adc_max_current) {
            return 100u;
        }

        percent = (adc - adc_no_current) * 100;
        percent = percent / (adc_max_current - adc_no_current);
    }
    /* Caso invertido: el ADC disminuye cuando aumenta la corriente */
    else {
        if (adc >= adc_no_current) {
            return 0u;
        }

        if (adc <= adc_max_current) {
            return 100u;
        }

        percent = (adc_no_current - adc) * 100;
        percent = percent / (adc_no_current - adc_max_current);
    }

    return (uint8_t)percent;
}

void task_pump_current_init(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;

    task_pump_current_data.state = TASK_PUMP_CURRENT_ST_WAIT_NEXT_SAMPLE;
    task_pump_current_data.sample_tick_count = PUMP_CURRENT_SAMPLE_TICKS;

    shared_data->pump_current_percent = 0u;
}

void task_pump_current_update(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;
    HAL_StatusTypeDef hal_status;
    ADC_ChannelConfTypeDef sConfig = {0};
    uint16_t adc_value;

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

            shared_data->pump_current_percent = task_pump_current_adc_to_percent(adc_value);

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
