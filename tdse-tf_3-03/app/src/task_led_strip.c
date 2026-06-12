#include "main.h"
#include "task_actuator_interface.h"
#include "task_led_strip.h"

// prototipos de funciones privadas
static void led_strip_off(void);
static void led_strip_on(void);
static void task_led_strip_statechart(void);

static task_actuator_dta_t led_strip_dta;

void task_led_strip_init(void *parameters){
	led_strip_off();
	led_strip_dta.event = EV_LED_STRIP_OFF;
	led_strip_dta.state = ST_LED_STRIP_OFF;
	led_strip_dta.event_pending = false;
}

void task_led_strip_update(void *parameters) {
    if (any_event_task_actuator(ID_ACT_LED_STRIP)) {
    	led_strip_dta.event = (task_led_strip_ev_t)get_event_task_actuator(ID_ACT_LED_STRIP);
    	led_strip_dta.event_pending = true;
    }

    task_led_strip_statechart();
}

static void task_led_strip_statechart(void){
	if (led_strip_dta.event_pending) {
		led_strip_dta.event_pending = false;

		switch(led_strip_dta.event) {
			case EV_LED_STRIP_ON:
				led_strip_on();
				led_strip_dta.state = ST_LED_STRIP_ON;
				break;
			case EV_LED_STRIP_OFF:
				led_strip_off();
				led_strip_dta.state = ST_LED_STRIP_OFF;
				break;
			default:
				break;
		}
	}

}

static void led_strip_off(void){
	HAL_GPIO_WritePin(Tira_LED_GPIO_Port, Tira_LED_Pin, GPIO_PIN_RESET);
}

static void led_strip_on(void){
	HAL_GPIO_WritePin(Tira_LED_GPIO_Port, Tira_LED_Pin, GPIO_PIN_SET);
}
