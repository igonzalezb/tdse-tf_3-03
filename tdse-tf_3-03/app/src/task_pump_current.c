/*
 * task_pump_current_it.c
 *
 * Version alternativa de task_pump_current.c usando interrupcion del ADC.
 *
 * Cambios principales respecto a la version con polling:
 * - Reemplaza HAL_ADC_Start() por HAL_ADC_Start_IT().
 * - El fin de conversion se atiende mediante task_pump_current_adc_conversion_complete(),
 *   llamada desde un unico router de callbacks del ADC.
 * - El task ya no llama a HAL_ADC_PollForConversion().
 * - Se agrega un timeout simple para no dejar tomado el ADC si falla la interrupcion.
 *
 * Importante:
 * Para usar varios tasks ADC por interrupcion, compilar tambien task_adc_callback_router.c
 * y no definir HAL_ADC_ConvCpltCallback() en cada task por separado.
 */
#include <stdbool.h>
#include "main.h"
#include "app.h"
#include "board.h"
#include "task_pump_current.h"
#include "task_menu_attribute.h"
#include "task_menu_interface.h"
//#include "task_system_failure.h"

extern ADC_HandleTypeDef hadc1;
extern shared_data_type shared_data;

// TODO Establecer maximos y minimos con task_sys_failure
// TODO Gestionar bien el llamado a modo falla

/*
 * Constantes de calibracion para convertir la lectura ADC a porcentaje.
 * Ajustar estos valores midiendo en Live Expressions:
 * - PUMP_CURRENT_ADC_NO_CURRENT: bomba apagada / sin corriente
 * - PUMP_CURRENT_ADC_MAX_CURRENT: bomba encendida en condicion normal/maxima
 */
#define PUMP_CURRENT_ADC_NO_CURRENT       0u
#define PUMP_CURRENT_ADC_REF_VALUE        1556u
#define PUMP_CURRENT_MA_REF_VALUE_X10     1285u

/*
 * Timeout de seguridad para no quedar esperando eternamente una interrupcion.
 * Si el scheduler corre cada 1 ms, 10 ticks ~= 10 ms.
 */
#define PUMP_CURRENT_ADC_TIMEOUT_TICKS    10u

/*
 * Limites para generar falla por sobrecorriente.
 * Ajustar segun calibracion real del sensor de corriente.
 * Se usa histeresis para no generar eventos repetidos cerca del limite.
 */
//#define PUMP_CURRENT_OVERCURRENT_LIMIT_PERCENT    90u
//#define PUMP_CURRENT_OVERCURRENT_CLEAR_PERCENT    80u  Si se usa histeresis

typedef enum {
    TASK_PUMP_CURRENT_ST_WAIT_NEXT_SAMPLE = 0,
    TASK_PUMP_CURRENT_ST_WAIT_ADC_CONVERSION
} task_pump_current_state_t;

typedef struct {
    task_pump_current_state_t state;
    uint32_t sample_tick_count;
    uint32_t adc_wait_tick_count;
} task_pump_current_data_t;

static task_pump_current_data_t task_pump_current_data;

/*
 * Estas variables las escribe la interrupcion y las lee el task.
 * Por eso son volatile.
 */
static volatile bool task_pump_current_adc_ready = false;
static volatile bool task_pump_current_adc_error_flag = false;
static volatile uint16_t task_pump_current_adc_value = 0u;

/*
static bool task_pump_current_overcurrent_active = false;

static uint8_t task_pump_current_adc_to_percent(uint16_t adc_value)
{
    int32_t percent;
    int32_t adc_no_current = (int32_t)PUMP_CURRENT_ADC_NO_CURRENT;
    int32_t adc_max_current = (int32_t)PUMP_CURRENT_ADC_MAX_CURRENT;
    int32_t adc = (int32_t)adc_value;

    if (adc_no_current == adc_max_current) {
        return 0u;
    }
*/
    /* Caso normal: el ADC aumenta cuando aumenta la corriente *//*
    if (adc_max_current > adc_no_current) {
        if (adc <= adc_no_current) {
            return 0u;
        }

        if (adc >= adc_max_current) {
            return 100u;
        }

        percent = (adc - adc_no_current) * 100;
        percent = percent / (adc_max_current - adc_no_current);
    }*/
    /* Caso invertido: el ADC disminuye cuando aumenta la corriente *//*
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
*/
static uint16_t task_pump_current_adc_to_ma(uint16_t adc_value)
{
    uint32_t adc = (uint32_t)adc_value;
    uint32_t adc_no_current = (uint32_t)PUMP_CURRENT_ADC_NO_CURRENT;
    uint32_t current_ma_x10;

    if (adc <= adc_no_current) {
        return 0u;
    }

    /* Regla de 3 simple: (ADC_Leido * 128.5) / 1556 */
    current_ma_x10 = ((adc - adc_no_current) * PUMP_CURRENT_MA_REF_VALUE_X10) / PUMP_CURRENT_ADC_REF_VALUE;

    /* Dividimos por 10 para devolver los mA exactos en formato entero */
    return (uint16_t)(current_ma_x10 / 10u);
}
void task_pump_current_init(void *parameters)
{
    shared_data_type *shared_data_ptr = (shared_data_type *) parameters;

    task_pump_current_data.state = TASK_PUMP_CURRENT_ST_WAIT_NEXT_SAMPLE;
    task_pump_current_data.sample_tick_count = PUMP_CURRENT_SAMPLE_TICKS;
    task_pump_current_data.adc_wait_tick_count = 0u;

    task_pump_current_adc_ready = false;
    task_pump_current_adc_error_flag = false;
    task_pump_current_adc_value = 0u;

    shared_data_ptr->pump_current_ma = 0u;
}

void task_pump_current_update(void *parameters)
{
    shared_data_type *shared_data_ptr = (shared_data_type *) parameters;
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

        if (shared_data_ptr->adc_busy == true) {
            break;
        }

        sConfig.Channel = PUMP_CURRENT_ADC_CHANNEL;
        sConfig.Rank = ADC_REGULAR_RANK_1;
        sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

        hal_status = HAL_ADC_ConfigChannel(&hadc1, &sConfig);
        if (hal_status != HAL_OK) {
            break;
        }

        task_pump_current_adc_ready = false;
        task_pump_current_adc_error_flag = false;
        task_pump_current_data.adc_wait_tick_count = 0u;

        shared_data_ptr->adc_busy = true;
        shared_data_ptr->adc_owner = ADC_OWNER_PUMP_CURRENT;

        /* Inicio no bloqueante: el ADC avisa por interrupcion cuando termina. */
        hal_status = HAL_ADC_Start_IT(&hadc1);
        if (hal_status == HAL_OK) {
            task_pump_current_data.state = TASK_PUMP_CURRENT_ST_WAIT_ADC_CONVERSION;
        } else {
            shared_data_ptr->adc_busy = false;
            shared_data_ptr->adc_owner = ADC_OWNER_NONE;
            task_pump_current_data.state = TASK_PUMP_CURRENT_ST_WAIT_NEXT_SAMPLE;
        }

        break;

    case TASK_PUMP_CURRENT_ST_WAIT_ADC_CONVERSION:

        if (shared_data_ptr->adc_owner != ADC_OWNER_PUMP_CURRENT) {
            task_pump_current_adc_ready = false;
            task_pump_current_adc_error_flag = false;
            task_pump_current_data.state = TASK_PUMP_CURRENT_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        if (task_pump_current_adc_ready == true) {
            adc_value = task_pump_current_adc_value;
            task_pump_current_adc_ready = false;

            HAL_ADC_Stop_IT(&hadc1);

            shared_data_ptr->adc_busy = false;
            shared_data_ptr->adc_owner = ADC_OWNER_NONE;

            shared_data_ptr->pump_current_ma = task_pump_current_adc_to_ma(adc_value);
            /*
             * Generacion de falla generica para el menu.
             * El evento es generico: EV_SYS_FAILURE.
             * El origen especifico queda indicado por shared_data_ptr->pump_current_failure.

            if ((task_pump_current_overcurrent_active == false) &&
                (shared_data_ptr->pump_current_percent >= PUMP_CURRENT_OVERCURRENT_LIMIT_PERCENT)) {

                task_pump_current_overcurrent_active = true;
                shared_data_ptr->pump_current_failure = true;

                put_event_task_menu(EV_SYS_FAILURE);
            }
			 */

            /* Con esto comentado, el sensor no baja el flag de falla. Se lo deja al modo falla
             *
            if ((task_pump_current_overcurrent_active == true) &&
                (shared_data_ptr->pump_current_percent <= PUMP_CURRENT_OVERCURRENT_CLEAR_PERCENT)) {

                task_pump_current_overcurrent_active = false;
                shared_data_ptr->pump_current_failure = false;
            }
			*/

            task_pump_current_data.state = TASK_PUMP_CURRENT_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        if (task_pump_current_adc_error_flag == true) {
            task_pump_current_adc_error_flag = false;

            HAL_ADC_Stop_IT(&hadc1);

            shared_data_ptr->adc_busy = false;
            shared_data_ptr->adc_owner = ADC_OWNER_NONE;

            task_pump_current_data.state = TASK_PUMP_CURRENT_ST_WAIT_NEXT_SAMPLE;
            break;
        }

        /* Timeout de seguridad por si no llega nunca HAL_ADC_ConvCpltCallback(). */
        task_pump_current_data.adc_wait_tick_count++;
        if (task_pump_current_data.adc_wait_tick_count >= PUMP_CURRENT_ADC_TIMEOUT_TICKS) {
            HAL_ADC_Stop_IT(&hadc1);

            shared_data_ptr->adc_busy = false;
            shared_data_ptr->adc_owner = ADC_OWNER_NONE;

            task_pump_current_data.state = TASK_PUMP_CURRENT_ST_WAIT_NEXT_SAMPLE;
        }

        break;

    default:
        task_pump_current_data.state = TASK_PUMP_CURRENT_ST_WAIT_NEXT_SAMPLE;
        task_pump_current_data.sample_tick_count = 0u;
        task_pump_current_data.adc_wait_tick_count = 0u;
        break;
    }
}


void task_pump_current_adc_conversion_complete(uint16_t adc_value)
{
    task_pump_current_adc_value = adc_value;
    task_pump_current_adc_ready = true;
}

void task_pump_current_adc_error(void)
{
    task_pump_current_adc_error_flag = true;
}
