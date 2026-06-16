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

/*
 * Calibracion del sensor de corriente del LED:
 *
 * LED_CURRENT_ADC_NO_CURRENT:
 *   Valor ADC medido cuando el LED esta apagado o no circula corriente.
 *
 * LED_CURRENT_ADC_MAX_CURRENT:
 *   Valor ADC medido con la corriente maxima que quieras tomar como 100%.
 *
 * Estos valores son de ejemplo. Reemplazalos por los que midas
 * temporalmente durante la calibracion, si necesitás ajustar la escala.
 *
 * La funcion de conversion soporta ambos casos:
 *   - ADC mas alto con mas corriente
 *   - ADC mas bajo con mas corriente
 */
#define LED_CURRENT_ADC_NO_CURRENT     0u
#define LED_CURRENT_ADC_MAX_CURRENT    4095u

typedef enum {
    TASK_LED_CURRENT_ST_WAIT_NEXT_SAMPLE = 0,
    TASK_LED_CURRENT_ST_WAIT_ADC_CONVERSION
} task_led_current_state_t;

typedef struct {
    task_led_current_state_t state;
    uint32_t sample_tick_count;
} task_led_current_data_t;

static task_led_current_data_t task_led_current_data;

static uint16_t task_led_current_adc_to_percent(uint16_t adc_value)
{
    int32_t adc_no_current = (int32_t) LED_CURRENT_ADC_NO_CURRENT;
    int32_t adc_max_current = (int32_t) LED_CURRENT_ADC_MAX_CURRENT;
    int32_t adc = (int32_t) adc_value;
    int32_t percent;

    if (adc_no_current == adc_max_current) {
        return 0u;
    }

    /*
     * Caso 1:
     *   Sin corriente -> ADC bajo
     *   Mas corriente -> ADC alto
     */
    if (adc_max_current > adc_no_current) {

        if (adc <= adc_no_current) {
            return 0u;
        }

        if (adc >= adc_max_current) {
            return 100u;
        }

        percent = ((adc - adc_no_current) * 100) /
                  (adc_max_current - adc_no_current);
    }

    /*
     * Caso 2:
     *   Sin corriente -> ADC alto
     *   Mas corriente -> ADC bajo
     */
    else {

        if (adc >= adc_no_current) {
            return 0u;
        }

        if (adc <= adc_max_current) {
            return 100u;
        }

        percent = ((adc_no_current - adc) * 100) /
                  (adc_no_current - adc_max_current);
    }

    if (percent < 0) {
        percent = 0;
    }
    else if (percent > 100) {
        percent = 100;
    }

    return (uint16_t) percent;
}

void task_led_current_init(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;

    task_led_current_data.state = TASK_LED_CURRENT_ST_WAIT_NEXT_SAMPLE;
    task_led_current_data.sample_tick_count = LED_CURRENT_SAMPLE_TICKS;

    shared_data->led_current_percent = 0u;
}

void task_led_current_update(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;
    HAL_StatusTypeDef hal_status;
    ADC_ChannelConfTypeDef sConfig = {0};
    uint16_t adc_value;
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

            shared_data->led_current_percent =
                (uint8_t) task_led_current_adc_to_percent(adc_value);

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

