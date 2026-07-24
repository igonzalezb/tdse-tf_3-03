#include "main.h"
//#include "app.h"
#include "task_buzzer.h"
#include "task_actuator_interface.h"
#include "task_actuator_attribute.h"


/**********************************************************************************************/

// Estructura de configuración de los patrones
typedef struct {
    task_buzzer_ev_t event;      // Evento que dispara este patrón
    buzzer_mode_t    mode;       // Modo operacional (Apagado, Fijo o Intermitente)
    uint32_t         ton_ms;     // Tiempo de encendido del pulso
    uint32_t         toff_ms;    // Tiempo de apagado entre pulsos
    uint16_t         pulses;     // Cantidad de pulsos (0 = intermitente)
} buzzer_pattern_t;


// Patrones para cada evento

static const buzzer_pattern_t buzzer_patterns[] = {
    // Evento,                  Modo,               Ton (ms)	Toff (ms) 	Pulses
    { EV_BUZZER_OFF,            BUZZER_MODE_OFF,    0,    		0,         	0 },  // Silencio total
    { EV_BUZZER_ON,             BUZZER_MODE_ON,     0,  		0,         	0 },  // Sonido continuo (Modo Test)
    { EV_BUZZER_1PULSE, 		BUZZER_MODE_BLINK,  80, 		0,			1 },  // 1 Beep corto
    { EV_BUZZER_INTERMITTENT,	BUZZER_MODE_BLINK,  300,		300,		0 },  // Intermitencia (Modo Falla)
    { EV_BUZZER_2PULSE, 		BUZZER_MODE_BLINK,  80,			80,			2 }   // 2 Beeps cortos
};

/**********************************************************************************************/


uint16_t const BUZZER_PATTERNS_QTY = (sizeof(buzzer_patterns) / sizeof(buzzer_patterns[0]));

// Variables estáticas de control de estado interno
static task_actuator_dta_t buzzer_dta;
static const buzzer_pattern_t *current_pattern = NULL;
static uint32_t remaining_pulses = 0;

// Prototipos de funciones internas de hardware y software
static void task_buzzer_statechart(void);
static void buzzer_on(void);
static void buzzer_off(void);


// Funciones auxiliares de abstracción de hardware
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
    current_pattern = NULL;
    remaining_pulses = 0;
    init_queue_event_task_actuator(ID_ACT_BUZZER);
}

void task_buzzer_update(void *parameters) {
    // Captura de eventos desde la cola genérica de actuadores
    if (any_event_task_actuator(ID_ACT_BUZZER)) {
        buzzer_dta.event = (task_buzzer_ev_t)get_event_task_actuator(ID_ACT_BUZZER);
        buzzer_dta.event_pending = true;
    }

    // Ejecución de la máquina de estados
    task_buzzer_statechart();
}

static void task_buzzer_statechart(void) {
    uint32_t current_tick = HAL_GetTick();

    if (buzzer_dta.event_pending) {
        buzzer_dta.event_pending = false;

        // Búsqueda del evento dentro de la tabla de patrones
        current_pattern = NULL;
        for (int i = 0; i < BUZZER_PATTERNS_QTY; i++) {
            if (buzzer_patterns[i].event == buzzer_dta.event) {
                current_pattern = &buzzer_patterns[i];
                break;
            }
        }

        // Si el evento existe en la tabla de configuración
		if (current_pattern != NULL) {
			if (current_pattern->mode == BUZZER_MODE_OFF) {
				buzzer_off();
				buzzer_dta.state = ST_BUZZER_IDLE;
			} else if (current_pattern->mode == BUZZER_MODE_ON) {
				buzzer_on();
				buzzer_dta.state = ST_BUZZER_ON;
			} else {
				// Modo Intermitente/Ráfagas: Encendemos el buzzer y seteamos los contadores
				buzzer_on();
				buzzer_dta.tick = current_tick;
				remaining_pulses = current_pattern->pulses;
				buzzer_dta.state = ST_BUZZER_INT_ON; // Usamos el estado genérico de sonido activo
			}
		} else {
            // Evento inválido o no configurado: Apagado por seguridad
            buzzer_off();
            buzzer_dta.state = ST_BUZZER_IDLE;
        }
        return;
    }

    // --- 2. TRANSICIONES DE TIEMPO (Máquina de Blink Genérica) ---
    switch (buzzer_dta.state) {
        case ST_BUZZER_IDLE:
        case ST_BUZZER_ON:
            break; // No requieren control temporal secuencial

        case ST_BUZZER_INT_ON: // Período en el que el sonido está activo
            if ((current_tick - buzzer_dta.tick) >= current_pattern->ton_ms) {
                buzzer_off();
                buzzer_dta.tick = current_tick;

                // Si es un patrón con cantidad de pulsos limitada, descontamos uno
                if (remaining_pulses > 0) {
                    remaining_pulses--;
                }

                // Si terminamos la ráfaga de pulsos, volvemos a IDLE
                if (current_pattern->pulses > 0 && remaining_pulses == 0) {
                    buzzer_dta.state = ST_BUZZER_IDLE;
                } else {
                    buzzer_dta.state = ST_BUZZER_INT_OFF;
                }
            }
            break;

        case ST_BUZZER_INT_OFF: // Período de silencio entre pulsos
            if ((current_tick - buzzer_dta.tick) >= current_pattern->toff_ms) {
                buzzer_on();
                buzzer_dta.tick = current_tick;
                buzzer_dta.state = ST_BUZZER_INT_ON;
            }
            break;

        default:
            break;
    }
}
