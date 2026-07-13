#ifndef APP_INC_APP_H_
#define APP_INC_APP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "task_menu_attribute.h"

typedef enum {
    ADC_OWNER_NONE = 0,
    ADC_OWNER_HUMIDITY,		//Para decidir quien esta utilizando el ADC1
    ADC_OWNER_LIGHT,
    ADC_OWNER_WATER_LEVEL,
	ADC_OWNER_PUMP_CURRENT,
	ADC_OWNER_LED_CURRENT
} adc_owner_t;

typedef struct {

    uint16_t humidity_percent;

    uint16_t light_percent;
    bool led_current_failure;

    uint16_t water_level_percent;
    bool water_level_failure;

    uint16_t pump_current_percent;
    bool pump_current_failure;

    uint16_t led_current_percent;

    uint16_t dht22_humidity;
    int16_t dht22_temperature;
    uint8_t dht22_error;

    bool adc_busy;  // Esta el adc usandose?
    adc_owner_t adc_owner; //Quien lo esta usando

    bool pump_on;
    bool led_strip_on;

    uint16_t config_values[CONFIG_WATER_LEVEL];

	// Menu: variables
	task_menu_sys_t active_system;

} shared_data_type;

extern shared_data_type shared_data;

extern uint32_t g_app_cnt; //
extern uint32_t g_app_time_us; //Estos 2 son para medir tiempod e ejecucion en dwt.h
extern uint32_t g_app_wcet_us; // Peor tiempo de ejecucion

void app_init(void);
void app_update(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_INC_APP_H_ */
