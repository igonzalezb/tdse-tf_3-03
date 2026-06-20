/*
 * task_adc_callback_router.c
 *
 * Router unico para callbacks del ADC cuando varios tasks usan HAL_ADC_Start_IT().
 *
 * Importante:
 * - En el proyecto debe existir una sola definicion de HAL_ADC_ConvCpltCallback().
 * - En el proyecto debe existir una sola definicion de HAL_ADC_ErrorCallback().
 * - No compilar otros archivos que tambien definan esos callbacks.
 */
#include "main.h"
#include "app.h"

extern shared_data_type shared_data;

void task_humidity_adc_conversion_complete(uint16_t adc_value);
void task_light_adc_conversion_complete(uint16_t adc_value);
void task_water_level_adc_conversion_complete(uint16_t adc_value);
void task_pump_current_adc_conversion_complete(uint16_t adc_value);
void task_led_current_adc_conversion_complete(uint16_t adc_value);

void task_humidity_adc_error(void);
void task_light_adc_error(void);
void task_water_level_adc_error(void);
void task_pump_current_adc_error(void);
void task_led_current_adc_error(void);

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    uint16_t adc_value;

    if (hadc->Instance != ADC1) {
        return;
    }

    adc_value = (uint16_t)HAL_ADC_GetValue(hadc);

    switch (shared_data.adc_owner)
    {
    case ADC_OWNER_HUMIDITY:
        task_humidity_adc_conversion_complete(adc_value);
        break;

    case ADC_OWNER_LIGHT:
        task_light_adc_conversion_complete(adc_value);
        break;

    case ADC_OWNER_WATER_LEVEL:
        task_water_level_adc_conversion_complete(adc_value);
        break;

    case ADC_OWNER_PUMP_CURRENT:
        task_pump_current_adc_conversion_complete(adc_value);
        break;

    case ADC_OWNER_LED_CURRENT:
        task_led_current_adc_conversion_complete(adc_value);
        break;

    case ADC_OWNER_NONE:
    default:
        break;
    }
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance != ADC1) {
        return;
    }

    switch (shared_data.adc_owner)
    {
    case ADC_OWNER_HUMIDITY:
        task_humidity_adc_error();
        break;

    case ADC_OWNER_LIGHT:
        task_light_adc_error();
        break;

    case ADC_OWNER_WATER_LEVEL:
        task_water_level_adc_error();
        break;

    case ADC_OWNER_PUMP_CURRENT:
        task_pump_current_adc_error();
        break;

    case ADC_OWNER_LED_CURRENT:
        task_led_current_adc_error();
        break;

    case ADC_OWNER_NONE:
    default:
        break;
    }
}
