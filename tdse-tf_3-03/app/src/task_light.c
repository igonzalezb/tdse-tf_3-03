#include "main.h"
#include "app.h"
#include "board.h"
#include "task_light.h"
#include <stdbool.h>

extern ADC_HandleTypeDef hadc1;

/*
 * Calibracion del sensor de luz analogico.
 *
 * LIGHT_ADC_DARK:
 *   Valor ADC medido con el sensor en oscuridad o con muy poca luz.
 *
 * LIGHT_ADC_BRIGHT:
 *   Valor ADC medido con el sensor con mucha luz.
 *
 * Estos valores son de ejemplo. Reemplazalos por los que midas
 * para tu sensor y tu conexion.
 *
 * La funcion de conversion soporta ambos casos:
 *   - ADC mas alto con mas luz
 *   - ADC mas bajo con mas luz
 */
#define LIGHT_ADC_DARK      4095u
#define LIGHT_ADC_BRIGHT    0u

typedef enum {
    TASK_LIGHT_ST_WAIT_NEXT_SAMPLE = 0,
    TASK_LIGHT_ST_WAIT_ADC_CONVERSION
} task_light_state_t;

typedef struct {
    task_light_state_t state;
    uint32_t sample_tick_count;
} task_light_data_t;

static task_light_data_t task_light_data;

static uint8_t task_light_adc_to_percent(uint16_t adc_value)
{
#if (LIGHT_ADC_BRIGHT == LIGHT_ADC_DARK)

    return 0u;

#elif (LIGHT_ADC_BRIGHT > LIGHT_ADC_DARK)

    /*
     * Caso 1:
     *   Oscuro   -> ADC bajo
     *   Luminoso -> ADC alto
     */
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

    /*
     * Caso 2:
     *   Oscuro   -> ADC alto
     *   Luminoso -> ADC bajo
     */
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

            shared_data->light_percent = task_light_adc_to_percent(adc_value);

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
