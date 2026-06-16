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

typedef enum {
    TASK_HUMIDITY_ST_WAIT_NEXT_SAMPLE = 0,
    TASK_HUMIDITY_ST_WAIT_ADC_CONVERSION
} task_humidity_state_t;

typedef struct {
    task_humidity_state_t state;
    uint32_t sample_tick_count;
} task_humidity_data_t;

static task_humidity_data_t task_humidity_data;

static uint8_t task_humidity_adc_to_percent(uint16_t adc_value)
{
    int32_t dry = (int32_t) YL69_ADC_DRY;
    int32_t wet = (int32_t) YL69_ADC_WET;
    int32_t adc = (int32_t) adc_value;
    int32_t percent;

    if (dry == wet) {
        return 0u;
    }

    /*
     * Caso usual del YL-69:
     * seco   -> ADC alto
     * mojado -> ADC bajo
     */
    if (dry > wet) {

        if (adc >= dry) {
            return 0u;
        }

        if (adc <= wet) {
            return 100u;
        }

        percent = ((dry - adc) * 100) / (dry - wet);
    }

    /*
     * Caso inverso, por si tu modulo/senal queda al reves:
     * seco   -> ADC bajo
     * mojado -> ADC alto
     */
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

    /*
     * Esta tarea solo publica el porcentaje de humedad.
     * No comparte el ADC crudo, thresholds ni booleanos de estado.
     */
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

        shared_data->adc_busy = true;
        shared_data->adc_owner = ADC_OWNER_HUMIDITY;

        hal_status = HAL_ADC_Start(&hadc1);
        if (hal_status == HAL_OK) {
            task_humidity_data.state = TASK_HUMIDITY_ST_WAIT_ADC_CONVERSION;
        }
        else {
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

            /*
             * Unico dato publicado por esta task.
             */
            shared_data->humidity_percent = task_humidity_adc_to_percent(adc_value);

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
