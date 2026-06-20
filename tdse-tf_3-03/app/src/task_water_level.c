/*
 * task_water_level_it.c
 *
 * Version de task_water_level.c usando interrupcion del ADC.
 */
#include <stdbool.h>
#include "main.h"
#include "app.h"
#include "board.h"
#include "task_water_level.h"

extern ADC_HandleTypeDef hadc1;

/*
 * Calibracion del sensor de nivel de agua:
 * - WATER_LEVEL_ADC_EMPTY: lectura ADC con el sensor seco / sin agua.
 * - WATER_LEVEL_ADC_FULL: lectura ADC al nivel maximo tomado como 100%.
 */
#define WATER_LEVEL_ADC_EMPTY    0u
#define WATER_LEVEL_ADC_FULL     2200u

/* Si el scheduler corre cada 1 ms, 10 ticks ~= 10 ms. */
#define WATER_LEVEL_ADC_TIMEOUT_TICKS    10u

typedef enum {
    TASK_WATER_LEVEL_ST_WAIT_NEXT_SAMPLE = 0,
    TASK_WATER_LEVEL_ST_WAIT_ADC_CONVERSION
} task_water_level_state_t;

typedef struct {
    task_water_level_state_t state;
    uint32_t sample_tick_count;
    uint32_t adc_wait_tick_count;
} task_water_level_data_t;

static task_water_level_data_t task_water_level_data;

/* Variables escritas por interrupcion y leidas por el task. */
static volatile bool task_water_level_adc_ready = false;
static volatile bool task_water_level_adc_error_flag = false;
static volatile uint16_t task_water_level_adc_value = 0u;

static uint8_t task_water_level_adc_to_percent(uint16_t adc_value)
{
    int32_t percent;

    if (WATER_LEVEL_ADC_FULL > WATER_LEVEL_ADC_EMPTY) {

        if (adc_value <= WATER_LEVEL_ADC_EMPTY) {
            return 0u;
        }

        if (adc_value >= WATER_LEVEL_ADC_FULL) {
            return 100u;
        }

        percent = ((int32_t)adc_value - (int32_t)WATER_LEVEL_ADC_EMPTY) * 100;
        percent = percent / ((int32_t)WATER_LEVEL_ADC_FULL - (int32_t)WATER_LEVEL_ADC_EMPTY);
    }
    else if (WATER_LEVEL_ADC_FULL < WATER_LEVEL_ADC_EMPTY) {

        if (adc_value >= WATER_LEVEL_ADC_EMPTY) {
            return 0u;
        }

        if (adc_value <= WATER_LEVEL_ADC_FULL) {
            return 100u;
        }

        percent = ((int32_t)WATER_LEVEL_ADC_EMPTY - (int32_t)adc_value) * 100;
        percent = percent / ((int32_t)WATER_LEVEL_ADC_EMPTY - (int32_t)WATER_LEVEL_ADC_FULL);
    }
    else {
        percent = 0;
    }

    if (percent < 0) {
        percent = 0;
    }
    else if (percent > 100) {
        percent = 100;
    }

    return (uint8_t)percent;
}

void task_water_level_init(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;

    task_water_level_data.state = TASK_WATER_LEVEL_ST_WAIT_NEXT_SAMPLE;
    task_water_level_data.sample_tick_count = WATER_LEVEL_SAMPLE_TICKS;
    task_water_level_data.adc_wait_tick_count = 0u;

    task_water_level_adc_ready = false;
    task_water_level_adc_error_flag = false;
    task_water_level_adc_value = 0u;

    shared_data->water_level_percent = 0u;
}

void task_water_level_update(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;
    HAL_StatusTypeDef hal_status;
    ADC_ChannelConfTypeDef sConfig = {0};
    uint16_t adc_value;

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

        task_water_level_adc_ready = false;
        task_water_level_adc_error_flag = false;
        task_water_level_data.adc_wait_tick_count = 0u;

        shared_data->adc_busy = true;
        shared_data->adc_owner = ADC_OWNER_WATER_LEVEL;

        /* Inicio no bloqueante: el ADC avisa por interrupcion cuando termina. */
        hal_status = HAL_ADC_Start_IT(&hadc1);
        if (hal_status == HAL_OK) {
            task_water_level_data.state = TASK_WATER_LEVEL_ST_WAIT_ADC_CONVERSION;
        }
        else {
            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;
            task_water_level_data.state = TASK_WATER_LEVEL_ST_WAIT_NEXT_SAMPLE;
        }

        break;

    case TASK_WATER_LEVEL_ST_WAIT_ADC_CONVERSION:

        if (shared_data->adc_owner != ADC_OWNER_WATER_LEVEL) {
            task_water_level_adc_ready = false;
            task_water_level_adc_error_flag = false;
            task_water_level_data.state = TASK_WATER_LEVEL_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        if (task_water_level_adc_ready == true) {
            adc_value = task_water_level_adc_value;
            task_water_level_adc_ready = false;

            HAL_ADC_Stop_IT(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            shared_data->water_level_percent = task_water_level_adc_to_percent(adc_value);

            task_water_level_data.state = TASK_WATER_LEVEL_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        if (task_water_level_adc_error_flag == true) {
            task_water_level_adc_error_flag = false;

            HAL_ADC_Stop_IT(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            task_water_level_data.state = TASK_WATER_LEVEL_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        task_water_level_data.adc_wait_tick_count++;
        if (task_water_level_data.adc_wait_tick_count >= WATER_LEVEL_ADC_TIMEOUT_TICKS) {
            HAL_ADC_Stop_IT(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            task_water_level_data.state = TASK_WATER_LEVEL_ST_WAIT_NEXT_SAMPLE;
        }

        break;

    default:
        task_water_level_data.state = TASK_WATER_LEVEL_ST_WAIT_NEXT_SAMPLE;
        task_water_level_data.sample_tick_count = 0u;
        task_water_level_data.adc_wait_tick_count = 0u;
        break;
    }
}

void task_water_level_adc_conversion_complete(uint16_t adc_value)
{
    task_water_level_adc_value = adc_value;
    task_water_level_adc_ready = true;
}

void task_water_level_adc_error(void)
{
    task_water_level_adc_error_flag = true;
}
