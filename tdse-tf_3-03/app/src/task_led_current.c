/*
 * task_led_current_it.c
 *
 * Version de task_led_current.c usando interrupcion del ADC.
 */
#include <stdbool.h>
#include "main.h"
#include "app.h"
#include "board.h"
#include "task_led_current.h"
#include "task_menu_attribute.h"
#include "task_menu_interface.h"

// TODO Establecer maximos y minimos con task_sys_failure
// TODO Gestionar bien el llamado a modo falla

extern ADC_HandleTypeDef hadc1;

/*
 * Calibracion del sensor de corriente del LED:
 * - LED_CURRENT_ADC_NO_CURRENT: valor ADC sin corriente.
 * - LED_CURRENT_ADC_MAX_CURRENT: valor ADC con corriente maxima tomada como 100%.
 */
#define LED_CURRENT_ADC_NO_CURRENT       0u
#define LED_CURRENT_ADC_REF_VALUE        573u
#define LED_CURRENT_MA_REF_VALUE_X10     1636u

/* Si el scheduler corre cada 1 ms, 10 ticks ~= 10 ms. */
#define LED_CURRENT_ADC_TIMEOUT_TICKS    10u

/* Limites para generar falla por sobrecorriente de LED.
 * Cuando supera LIMIT se genera EV_SYS_FAILURE.
 * Cuando baja de CLEAR se limpia el flag de falla.
 */
//#define LED_CURRENT_OVERCURRENT_LIMIT_PERCENT    80u
//#define LED_CURRENT_OVERCURRENT_CLEAR_PERCENT    70u  Si se usa histeresis

typedef enum {
    TASK_LED_CURRENT_ST_WAIT_NEXT_SAMPLE = 0,
    TASK_LED_CURRENT_ST_WAIT_ADC_CONVERSION
} task_led_current_state_t;

typedef struct {
    task_led_current_state_t state;
    uint32_t sample_tick_count;
    uint32_t adc_wait_tick_count;
} task_led_current_data_t;

static task_led_current_data_t task_led_current_data;

/* Variables escritas por interrupcion y leidas por el task. */
static volatile bool task_led_current_adc_ready = false;
static volatile bool task_led_current_adc_error_flag = false;
static volatile uint16_t task_led_current_adc_value = 0u;

static uint16_t task_led_current_adc_to_ma(uint16_t adc_value)
{
    uint32_t adc = (uint32_t)adc_value;
    uint32_t adc_no_current = (uint32_t)LED_CURRENT_ADC_NO_CURRENT;
    uint32_t current_ma_x10;

    if (adc <= adc_no_current) {
        return 0u;
    }

    /* Regla de 3 simple: (ADC_Leido * 163.6) / 573 */
    current_ma_x10 = ((adc - adc_no_current) * LED_CURRENT_MA_REF_VALUE_X10) / LED_CURRENT_ADC_REF_VALUE;

    /* Dividimos por 10 para devolver los mA exactos en formato entero */
    return (uint16_t)(current_ma_x10 / 10u);
}

void task_led_current_init(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;

    task_led_current_data.state = TASK_LED_CURRENT_ST_WAIT_NEXT_SAMPLE;
    task_led_current_data.sample_tick_count = LED_CURRENT_SAMPLE_TICKS;
    task_led_current_data.adc_wait_tick_count = 0u;

    task_led_current_adc_ready = false;
    task_led_current_adc_error_flag = false;
    task_led_current_adc_value = 0u;

    shared_data->led_current_ma = 0u;
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

        task_led_current_adc_ready = false;
        task_led_current_adc_error_flag = false;
        task_led_current_data.adc_wait_tick_count = 0u;

        shared_data->adc_busy = true;
        shared_data->adc_owner = ADC_OWNER_LED_CURRENT;

        /* Inicio no bloqueante: el ADC avisa por interrupcion cuando termina. */
        hal_status = HAL_ADC_Start_IT(&hadc1);
        if (hal_status == HAL_OK) {
            task_led_current_data.state = TASK_LED_CURRENT_ST_WAIT_ADC_CONVERSION;
        }
        else {
            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;
            task_led_current_data.state = TASK_LED_CURRENT_ST_WAIT_NEXT_SAMPLE;
        }

        break;

    case TASK_LED_CURRENT_ST_WAIT_ADC_CONVERSION:

        if (shared_data->adc_owner != ADC_OWNER_LED_CURRENT) {
            task_led_current_adc_ready = false;
            task_led_current_adc_error_flag = false;
            task_led_current_data.state = TASK_LED_CURRENT_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        if (task_led_current_adc_ready == true) {
            adc_value = task_led_current_adc_value;
            task_led_current_adc_ready = false;

            HAL_ADC_Stop_IT(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            shared_data->led_current_ma = task_led_current_adc_to_ma(adc_value);

            task_led_current_data.state = TASK_LED_CURRENT_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        if (task_led_current_adc_error_flag == true) {
            task_led_current_adc_error_flag = false;

            HAL_ADC_Stop_IT(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            task_led_current_data.state = TASK_LED_CURRENT_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        task_led_current_data.adc_wait_tick_count++;
        if (task_led_current_data.adc_wait_tick_count >= LED_CURRENT_ADC_TIMEOUT_TICKS) {
            HAL_ADC_Stop_IT(&hadc1);

            shared_data->adc_busy = false;
            shared_data->adc_owner = ADC_OWNER_NONE;

            task_led_current_data.state = TASK_LED_CURRENT_ST_WAIT_NEXT_SAMPLE;
        }

        break;

    default:
        task_led_current_data.state = TASK_LED_CURRENT_ST_WAIT_NEXT_SAMPLE;
        task_led_current_data.sample_tick_count = 0u;
        task_led_current_data.adc_wait_tick_count = 0u;
        break;
    }
}

void task_led_current_adc_conversion_complete(uint16_t adc_value)
{
    task_led_current_adc_value = adc_value;
    task_led_current_adc_ready = true;
}

void task_led_current_adc_error(void)
{
    task_led_current_adc_error_flag = true;
}
