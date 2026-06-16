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

typedef enum task_state_led_blinking {
	STATE_LED_NO_BLINK,
	STATE_LED_SLOW_BLINK,
	STATE_LED_FAST_BLINK,
} task_state_led_blinking_t;

typedef enum task_buzzer_mode{
    BUZZER_MODE_OFF,
    BUZZER_MODE_PULSE,         // Un solo beep corto al cambiar de menú
    BUZZER_MODE_INTERMITTENT   // Alarma continua para error
} task_buzzer_mode_t;

typedef struct {

    uint8_t humidity_percent;

    uint16_t light_percent;

    uint16_t water_level_percent;

    uint16_t pump_current_percent;

    uint16_t led_current_percent;

    uint16_t dht22_humidity;
    int16_t  dht22_temperature;
    uint8_t dht22_error;

    bool adc_busy;  // Esta el adc usandose?
    adc_owner_t adc_owner; //Quien lo esta usando

    //info del led de estado
    uint16_t pwm_state_led_red;
	uint16_t pwm_state_led_green;
	uint16_t pwm_state_led_blue;
	task_state_led_blinking_t blinking_rate;
	bool state_led_data_changed;

	// info del buzzer
	task_buzzer_mode_t buzzer_mode;
	bool buzzer_mode_changed;

	// Menu: variables
	task_menu_sys_t active_system;

	task_menu_test_t active_test;



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
