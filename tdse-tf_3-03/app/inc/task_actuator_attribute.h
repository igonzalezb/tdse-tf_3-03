#ifndef TASK_INC_TASK_ACTUATOR_ATTRIBUTE_H_
#define TASK_INC_TASK_ACTUATOR_ATTRIBUTE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Identificadores únicos para cada actuador */
typedef enum task_actuator_id{
    ID_ACT_BUZZER = 0,
	ID_ACT_STATE_LED,
	ID_ACT_PUMP,
	ID_ACT_LED_STRIP,
	ID_ACT_QTY // Cantidad total de actuadores
} task_actuator_id_t;

/* Eventos y estados específicos para el BUZZER */
typedef enum task_buzzer_ev{
    EV_BUZZER_OFF = 0,				// Apagar el buzzer (modo silencioso)
	EV_BUZZER_ON,					// Encender el buzzer (modo test)
    EV_BUZZER_PULSE,				// Generar un pulso de buzzer
    EV_BUZZER_INTERMITTENT			// Generar intermitencia por tiempo indefinido, (modo falla)
} task_buzzer_ev_t;

typedef enum task_buzzer_st{
    ST_BUZZER_IDLE,
    ST_BUZZER_PULSE_ON,
    ST_BUZZER_INT_ON,
    ST_BUZZER_INT_OFF,
	ST_BUZZER_ON
} task_buzzer_st_t;


/* Eventos específicos para el STATE_LED */
typedef enum task_state_led_ev{
    EV_STATE_LED_SYS_NORMAL,
	EV_STATE_LED_SYS_SETUP,
    EV_STATE_LED_SYS_FAILURE,
    EV_STATE_LED_SYS_TEST,
	EV_STATE_LED_ON					// Encender el led (modo test)
} task_state_led_ev_t;


typedef enum task_pump_ev{
    EV_PUMP_OFF = 0,
	EV_PUMP_ON
} task_pump_ev_t;

typedef enum task_led_strip_ev{
    EV_LED_STRIP_OFF = 0,
	EV_LED_STRIP_ON
} task_led_strip_ev_t;

typedef struct {
    uint32_t state;           // Estado actual de la máquina de estados
    uint32_t event;           // Último evento recibido de la cola
    bool     event_pending;   // Bandera que avisa si hay un nuevo evento por procesar
    uint32_t tick;            // Temporizador general para no usar delays
} task_actuator_dta_t;



#ifdef __cplusplus
}
#endif

#endif /* TASK_INC_TASK_ACTUATOR_ATTRIBUTE_H_ */
