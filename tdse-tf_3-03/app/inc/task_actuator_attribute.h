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

/*==============================================================================================*/
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
/*==============================================================================================*/


/*==============================================================================================*/
/* Eventos específicos para el STATE_LED */
typedef enum task_state_led_ev{
    EV_STATE_LED_SYS_NORMAL,
	EV_STATE_LED_SYS_SETUP,
    EV_STATE_LED_SYS_FAILURE,
    EV_STATE_LED_SYS_TEST,
	EV_STATE_LED_ON,				// Encender el led (modo test)
	EV_STATE_LED_WATER				// Regar
} task_state_led_ev_t;

typedef enum task_state_led_st{
    ST_STATE_LED_IDLE,
    ST_STATE_LED_ON,
    ST_STATE_LED_BLINK_ON,
    ST_STATE_LED_BLINK_OFF
} task_state_led_st_t;

typedef struct {
    task_state_led_ev_t event;           	// Evento que dispara este patrón
    uint16_t            red;        		// Intensidad Rojo (0-100)
    uint16_t            green;      		// Intensidad Verde (0-100)
    uint16_t            blue;       		// Intensidad Azul (0-100)
    uint32_t            blinking_period;	// Tiempo de parpadeo (0 = Fijo)
} state_led_pattern_t;

// Tiempos de parpadeo
typedef enum blinking_period_ms {
	STATE_LED_NO_BLINK   = 0,
	STATE_LED_FAST_BLINK = 150,
	STATE_LED_SLOW_BLINK = 500
} blinking_period_ms_t;

/**********************************************************************************************/
// Patrones para cada evento
static const state_led_pattern_t led_patterns[] = {
    // Evento,                  R,   	G,   	B,  	Blink_ms
    { EV_STATE_LED_SYS_NORMAL,	0,   	100, 	0,  	STATE_LED_NO_BLINK   },	// Verde Fijo
    { EV_STATE_LED_SYS_SETUP,	0,   	0,   	75, 	STATE_LED_SLOW_BLINK }, // Azul Lento
    { EV_STATE_LED_SYS_FAILURE,	100,	0,   	0,  	STATE_LED_FAST_BLINK }, // Rojo Rápido
    { EV_STATE_LED_SYS_TEST,	50,  	0,   	50, 	STATE_LED_NO_BLINK   }, // Violeta Fijo
    { EV_STATE_LED_ON,			100, 	100, 	100,	STATE_LED_NO_BLINK   }, // Blanco Fijo
    { EV_STATE_LED_WATER, 		17,   	95, 	100,	STATE_LED_FAST_BLINK }  // Celeste regante
};
/**********************************************************************************************/

/*==============================================================================================*/



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
