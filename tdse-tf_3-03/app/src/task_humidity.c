/*
 * task_humidity_it.c
 *
 * Version de task_humidity.c usando interrupcion del ADC.
 *
 * Cambios respecto a la version con polling:
 * - Reemplaza HAL_ADC_Start() por HAL_ADC_Start_IT().
 * - El task ya no llama a HAL_ADC_PollForConversion().
 * - El resultado lo recibe mediante task_humidity_adc_conversion_complete(),
 *   llamada desde un unico router de callbacks del ADC.
 * - Agrega timeout simple para liberar el ADC si la interrupcion no llega.
 */
#include <main.h>
#include <stdbool.h>
#include "app.h"
#include "board.h"
#include "task_humidity.h"

extern ADC_HandleTypeDef hadc1;

/*
 * Calibracion YL-69:
 * - YL69_ADC_DRY: valor ADC medido con la tierra seca.
 * - YL69_ADC_WET: valor ADC medido con la tierra muy humeda/saturada.
 *
 * En la mayoria de los modulos YL-69:
 * - tierra seca   -> ADC mas alto
 * - tierra humeda -> ADC mas bajo
 *
 * Ajustar estos valores segun lo que veas en Live Expressions.
 */
#define YL69_ADC_DRY      4095u
#define YL69_ADC_WET      1700u

/* Si el scheduler corre cada 1 ms, 10 ticks ~= 10 ms. */
#define HUMIDITY_ADC_TIMEOUT_TICKS    10u

typedef enum {
    TASK_HUMIDITY_ST_WAIT_NEXT_SAMPLE = 0,
    TASK_HUMIDITY_ST_WAIT_ADC_CONVERSION
} task_humidity_state_t;

typedef struct {
    task_humidity_state_t state;
    uint32_t sample_tick_count;
    uint32_t adc_wait_tick_count;
} task_humidity_data_t;

static task_humidity_data_t task_humidity_data;

/* Variables escritas por interrupcion y leidas por el task. */
static volatile bool task_humidity_adc_ready = false;
static volatile bool task_humidity_adc_error_flag = false;
static volatile uint16_t task_humidity_adc_value = 0u;

static uint8_t task_humidity_adc_to_percent(uint16_t adc_value)
{
    int32_t dry = (int32_t) YL69_ADC_DRY;
    int32_t wet = (int32_t) YL69_ADC_WET;
    int32_t adc = (int32_t) adc_value;
    int32_t percent;

    if (dry == wet) {
        return 0u;
    }

    /* Caso usual del YL-69: seco -> ADC alto, mojado -> ADC bajo. */
    if (dry > wet) {

        if (adc >= dry) {
            return 0u;
        }

        if (adc <= wet) {
            return 100u;
        }

        percent = ((dry - adc) * 100) / (dry - wet);
    }
    /* Caso inverso: seco -> ADC bajo, mojado -> ADC alto. */
    else {

        if (adc <= dry) {
            return 0u;
        }

        if (adc >= wet) {
            return 100u;
        }

        percent = ((adc - dry) * 100) / (wet - dry);
    }

    if (percent < 0) {
        percent = 0;
    }
    else if (percent > 100) {
        percent = 100;
    }

    return (uint8_t) percent;
}

void task_humidity_init(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;

    task_humidity_data.state = TASK_HUMIDITY_ST_WAIT_NEXT_SAMPLE;
    task_humidity_data.sample_tick_count = HUMIDITY_SAMPLE_TICKS;
    task_humidity_data.adc_wait_tick_count = 0u;

    task_humidity_adc_ready = false;
    task_humidity_adc_error_flag = false;
    task_humidity_adc_value = 0u;

    shared_data->humidity_percent = 0u;
}

void task_humidity_update(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;
    HAL_StatusTypeDef hal_status;
    uint16_t adc_value;
    ADC_ChannelConfTypeDef sConfig = {0};

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

        task_humidity_adc_ready = false;
        task_humidity_adc_error_flag = false;
        task_humidity_data.adc_wait_tick_count = 0u;

        shared_data->adc_busy = true;
        shared_data->adc_owner = ADC_OWNER_HUMIDITY;

        /* Inicio no bloqueante: el ADC avisa por interrupcion cuando termina. */
        hal_status = HAL_ADC_Start_IT(&hadc1);
        if (hal_status == HAL_OK) {
            task_humidity_data.state = TASK_HUMIDITY_ST_WAIT_ADC_CONVERSION;
        }
        else {
            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;
            task_humidity_data.state = TASK_HUMIDITY_ST_WAIT_NEXT_SAMPLE;
        }

        break;

    case TASK_HUMIDITY_ST_WAIT_ADC_CONVERSION:

        if (shared_data->adc_owner != ADC_OWNER_HUMIDITY) {
            task_humidity_adc_ready = false;
            task_humidity_adc_error_flag = false;
            task_humidity_data.state = TASK_HUMIDITY_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        if (task_humidity_adc_ready == true) {
            adc_value = task_humidity_adc_value;
            task_humidity_adc_ready = false;

            HAL_ADC_Stop_IT(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            shared_data->humidity_percent = task_humidity_adc_to_percent(adc_value);

            task_humidity_data.state = TASK_HUMIDITY_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        if (task_humidity_adc_error_flag == true) {
            task_humidity_adc_error_flag = false;

            HAL_ADC_Stop_IT(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            task_humidity_data.state = TASK_HUMIDITY_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        task_humidity_data.adc_wait_tick_count++;
        if (task_humidity_data.adc_wait_tick_count >= HUMIDITY_ADC_TIMEOUT_TICKS) {
            HAL_ADC_Stop_IT(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            task_humidity_data.state = TASK_HUMIDITY_ST_WAIT_NEXT_SAMPLE;
        }

        break;

    default:
        task_humidity_data.state = TASK_HUMIDITY_ST_WAIT_NEXT_SAMPLE;
        task_humidity_data.sample_tick_count = 0u;
        task_humidity_data.adc_wait_tick_count = 0u;
        break;
    }
}

void task_humidity_adc_conversion_complete(uint16_t adc_value)
{
    task_humidity_adc_value = adc_value;
    task_humidity_adc_ready = true;
}

void task_humidity_adc_error(void)
{
    task_humidity_adc_error_flag = true;
}
