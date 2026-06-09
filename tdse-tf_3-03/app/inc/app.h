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
    uint16_t humidity_adc_value; // shared_data.humidity_adc_value ---> valor de adc medido
    uint16_t humidity_threshold; // shared_data.humidity_threshold ---> valor de threshold para determinar si esta humedo
    bool humidity; //
    bool humidity_changed; //

    uint16_t light_adc_value;
    uint16_t light_threshold; // Lo mismo q en humidity
    bool light;
    bool light_changed;

    uint16_t water_level_adc_value;
    uint16_t water_level_threshold;
    bool water_level;
    bool water_level_changed;

    uint16_t pump_current_adc_value;
    uint16_t pump_current_threshold;
    bool pump_current;
    bool pump_current_changed;

    uint16_t led_current_adc_value;
    uint16_t led_current_threshold;
    bool led_current;
    bool led_current_changed;

    uint16_t dht22_humidity_x10;
    int16_t  dht22_temperature_x10;
    bool dht22_valid;
    bool dht22_changed;
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
