/*
 * task_light_it.c
 *
 * Version de task_light.c usando interrupcion del ADC.
 */
#include "main.h"
#include "app.h"
#include "board.h"
#include "task_light.h"
#include <stdbool.h>

extern ADC_HandleTypeDef hadc1;

/*
 * Calibracion del sensor de luz analogico.
 * LIGHT_ADC_DARK: valor ADC en oscuridad.
 * LIGHT_ADC_BRIGHT: valor ADC con mucha luz.
 */
#define LIGHT_ADC_DARK      4095u
#define LIGHT_ADC_BRIGHT    0u

/* Si el scheduler corre cada 1 ms, 10 ticks ~= 10 ms. */
#define LIGHT_ADC_TIMEOUT_TICKS    10u

typedef enum {
    TASK_LIGHT_ST_WAIT_NEXT_SAMPLE = 0,
    TASK_LIGHT_ST_WAIT_ADC_CONVERSION
} task_light_state_t;

typedef struct {
    task_light_state_t state;
    uint32_t sample_tick_count;
    uint32_t adc_wait_tick_count;
} task_light_data_t;

static task_light_data_t task_light_data;

/* Variables escritas por interrupcion y leidas por el task. */
static volatile bool task_light_adc_ready = false;
static volatile bool task_light_adc_error_flag = false;
static volatile uint16_t task_light_adc_value = 0u;

static uint8_t task_light_adc_to_percent(uint16_t adc_value)
{
#if (LIGHT_ADC_BRIGHT == LIGHT_ADC_DARK)

    return 0u;

#elif (LIGHT_ADC_BRIGHT > LIGHT_ADC_DARK)

    /* Oscuro -> ADC bajo, luminoso -> ADC alto. */
    if (adc_value <= LIGHT_ADC_DARK) {
        return 0u;
    }

    if (adc_value >= LIGHT_ADC_BRIGHT) {
        return 100u;
    }

    return (uint8_t)(
        (((uint32_t)adc_value - (uint32_t)LIGHT_ADC_DARK) * 100u) /
        ((uint32_t)LIGHT_ADC_BRIGHT - (uint32_t)LIGHT_ADC_DARK)
    );

#else

    /* Oscuro -> ADC alto, luminoso -> ADC bajo. */
    if (adc_value >= LIGHT_ADC_DARK) {
        return 0u;
    }

    if (adc_value <= LIGHT_ADC_BRIGHT) {
        return 100u;
    }

    return (uint8_t)(
        (((uint32_t)LIGHT_ADC_DARK - (uint32_t)adc_value) * 100u) /
        ((uint32_t)LIGHT_ADC_DARK - (uint32_t)LIGHT_ADC_BRIGHT)
    );

#endif
}

void task_light_init(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;

    task_light_data.state = TASK_LIGHT_ST_WAIT_NEXT_SAMPLE;
    task_light_data.sample_tick_count = LIGHT_SAMPLE_TICKS;
    task_light_data.adc_wait_tick_count = 0u;

    task_light_adc_ready = false;
    task_light_adc_error_flag = false;
    task_light_adc_value = 0u;

    shared_data->light_percent = 0u;
}

void task_light_update(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;
    HAL_StatusTypeDef hal_status;
    ADC_ChannelConfTypeDef sConfig = {0};
    uint16_t adc_value;

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

        task_light_adc_ready = false;
        task_light_adc_error_flag = false;
        task_light_data.adc_wait_tick_count = 0u;

        shared_data->adc_busy = true;
        shared_data->adc_owner = ADC_OWNER_LIGHT;

        /* Inicio no bloqueante: el ADC avisa por interrupcion cuando termina. */
        hal_status = HAL_ADC_Start_IT(&hadc1);
        if (hal_status == HAL_OK) {
            task_light_data.state = TASK_LIGHT_ST_WAIT_ADC_CONVERSION;
        }
        else {
            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;
            task_light_data.state = TASK_LIGHT_ST_WAIT_NEXT_SAMPLE;
        }

        break;

    case TASK_LIGHT_ST_WAIT_ADC_CONVERSION:

        if (shared_data->adc_owner != ADC_OWNER_LIGHT) {
            task_light_adc_ready = false;
            task_light_adc_error_flag = false;
            task_light_data.state = TASK_LIGHT_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        if (task_light_adc_ready == true) {
            adc_value = task_light_adc_value;
            task_light_adc_ready = false;

            HAL_ADC_Stop_IT(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            shared_data->light_percent = task_light_adc_to_percent(adc_value);

            task_light_data.state = TASK_LIGHT_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        if (task_light_adc_error_flag == true) {
            task_light_adc_error_flag = false;

            HAL_ADC_Stop_IT(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            task_light_data.state = TASK_LIGHT_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        task_light_data.adc_wait_tick_count++;
        if (task_light_data.adc_wait_tick_count >= LIGHT_ADC_TIMEOUT_TICKS) {
            HAL_ADC_Stop_IT(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            task_light_data.state = TASK_LIGHT_ST_WAIT_NEXT_SAMPLE;
        }

        break;

    default:
        task_light_data.state = TASK_LIGHT_ST_WAIT_NEXT_SAMPLE;
        task_light_data.sample_tick_count = 0u;
        task_light_data.adc_wait_tick_count = 0u;
        break;
    }
}

void task_light_adc_conversion_complete(uint16_t adc_value)
{
    task_light_adc_value = adc_value;
    task_light_adc_ready = true;
}

void task_light_adc_error(void)
{
    task_light_adc_error_flag = true;
}
