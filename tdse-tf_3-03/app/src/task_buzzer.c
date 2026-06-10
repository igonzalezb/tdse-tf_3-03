#include "main.h"
#include "app.h"
#include "task_buzzer.h"
#include "task_actuator_interface.h"
#include "task_actuator_attribute.h"

#define BUZZER_PULSE_MS        80
#define BUZZER_INTERMITTENT_MS 300


static task_actuator_dta_t buzzer_dta;

// Prototipos de funciones internas
static void task_buzzer_statechart(void);
static void task_buzzer_time_transitions(void);
static void buzzer_on(void);
static void buzzer_off(void);

// Funciones auxiliares de hardware
static void buzzer_on(void) {
    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET);
}

static void buzzer_off(void) {
    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
}


void task_buzzer_init(void *parameters) {
    buzzer_off();
    buzzer_dta.state = ST_BUZZER_IDLE;
    buzzer_dta.event_pending = false;
    buzzer_dta.tick = 0;
}

void task_buzzer_update(void *parameters) {

    if (any_event_task_actuator(ID_ACT_BUZZER)) {
        buzzer_dta.event = (task_buzzer_ev_t)get_event_task_actuator(ID_ACT_BUZZER);
        buzzer_dta.event_pending = true; // Levantamos la bandera
    }

    if (buzzer_dta.event_pending) {
    	task_buzzer_statechart();
    }
    else{
		task_buzzer_time_transitions();
	}
}


static void task_buzzer_statechart(void) {
    uint32_t current_tick = HAL_GetTick();

	buzzer_dta.event_pending = false;

	switch (buzzer_dta.event) {
		case EV_BUZZER_PULSE:
			buzzer_on();
			buzzer_dta.tick = current_tick;
			buzzer_dta.state = ST_BUZZER_PULSE_ON;
			break;

		case EV_BUZZER_INTERMITTENT:
			buzzer_on();
			buzzer_dta.tick = current_tick;
			buzzer_dta.state = ST_BUZZER_INT_ON;
			break;

		case EV_BUZZER_ON:
			buzzer_on();
			buzzer_dta.state = ST_BUZZER_ON;
			break;

		case EV_BUZZER_OFF:
		default:
			buzzer_off();
			buzzer_dta.state = ST_BUZZER_IDLE;
			break;
    }

}

static void task_buzzer_time_transitions(void){
	uint32_t current_tick = HAL_GetTick();

    switch (buzzer_dta.state) {
        case ST_BUZZER_IDLE:
            // No hace nada, espera eventos.
            break;

        case ST_BUZZER_ON:
			// No hace nada, espera eventos.
			break;

        case ST_BUZZER_PULSE_ON:
            if ((current_tick - buzzer_dta.tick) >= BUZZER_PULSE_MS) {
                buzzer_off();
                buzzer_dta.state = ST_BUZZER_IDLE;
            }
            break;

        case ST_BUZZER_INT_ON:
            if ((current_tick - buzzer_dta.tick) >= BUZZER_INTERMITTENT_MS) {
                buzzer_off();
                buzzer_dta.tick = current_tick;
                buzzer_dta.state = ST_BUZZER_INT_OFF;
            }
            break;

        case ST_BUZZER_INT_OFF:
            if ((current_tick - buzzer_dta.tick) >= BUZZER_INTERMITTENT_MS) {
                buzzer_on();
                buzzer_dta.tick = current_tick;
                buzzer_dta.state = ST_BUZZER_INT_ON;
            }
            break;
    }
}
