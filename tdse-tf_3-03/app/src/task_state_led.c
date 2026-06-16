#include "main.h"
//#include "app.h"
#include "task_state_led.h"
#include "task_actuator_interface.h"

// Definición de canales del timer 4
#define CHANNEL_LED_RED   TIM_CHANNEL_4
#define CHANNEL_LED_GREEN TIM_CHANNEL_3
#define CHANNEL_LED_BLUE  TIM_CHANNEL_1
#define TIMER_STATE_LED htim4

// Definición de tiempos para patrones del LED de estado
#define STATE_LED_NO_BLINK 0
#define STATE_LED_FAST_BLINK 150
#define STATE_LED_SLOW_BLINK 500

extern TIM_HandleTypeDef TIMER_STATE_LED;


/**********************************************************************************************/

// Estructura de configuración de los patrones
typedef struct {
    task_state_led_ev_t event;           	// Evento que dispara este patrón
    uint16_t            red;        		// Intensidad Rojo (0-100)
    uint16_t            green;      		// Intensidad Verde (0-100)
    uint16_t            blue;       		// Intensidad Azul (0-100)
    uint32_t            blinking_period;	// Tiempo de parpadeo (0 = Fijo)
} state_led_pattern_t;


// Patrones para cada evento
static const state_led_pattern_t led_patterns[] = {
    // Evento,                  R,   	G,   	B,  	Blink_ms
	{ EV_STATE_LED_OFF,			0, 		0, 		0,		STATE_LED_NO_BLINK   }, // Apagado
	{ EV_STATE_LED_ON,			100, 	100, 	100,	STATE_LED_NO_BLINK   }, // Blanco Fijo
    { EV_STATE_LED_SYS_NORMAL,	0,   	100, 	0,  	STATE_LED_NO_BLINK   },	// Verde Fijo
    { EV_STATE_LED_SYS_SETUP,	0,   	0,   	75, 	STATE_LED_SLOW_BLINK }, // Azul Lento
    { EV_STATE_LED_SYS_FAILURE,	100,	0,   	0,  	STATE_LED_FAST_BLINK }, // Rojo Rápido
    { EV_STATE_LED_SYS_TEST,	50,  	0,   	50, 	STATE_LED_NO_BLINK   }, // Violeta Fijo
    { EV_STATE_LED_WATER, 		17,   	95, 	100,	STATE_LED_FAST_BLINK }  // Celeste regante
};
/**********************************************************************************************/

// Variables estáticas
static task_actuator_dta_t state_led_dta;
static const state_led_pattern_t *current_pattern = NULL;

// Funciones internas
static void apply_led_colors(uint16_t, uint16_t, uint16_t);
static void task_state_led_statechart(void);

// Constantes
uint16_t const LED_PATTERNS_QTY = (sizeof(led_patterns) / sizeof(led_patterns[0]));


void task_state_led_init(void *parameters) {
    HAL_TIM_PWM_Start(&TIMER_STATE_LED, CHANNEL_LED_RED);
    HAL_TIM_PWM_Start(&TIMER_STATE_LED, CHANNEL_LED_GREEN);
    HAL_TIM_PWM_Start(&TIMER_STATE_LED, CHANNEL_LED_BLUE);

    apply_led_colors(0, 0, 0);

    state_led_dta.state = ST_STATE_LED_IDLE;
    state_led_dta.event_pending = false;
    current_pattern = NULL;
}

void task_state_led_update(void *parameters) {
    if (any_event_task_actuator(ID_ACT_STATE_LED)) {
        state_led_dta.event = get_event_task_actuator(ID_ACT_STATE_LED);
        state_led_dta.event_pending = true;
    }

    task_state_led_statechart();
}

static void task_state_led_statechart(void) {
    uint32_t current_tick = HAL_GetTick();

    if (state_led_dta.event_pending) {
        state_led_dta.event_pending = false;

        // Se busca el evento entre los patrones
        current_pattern = NULL;
        for (int i = 0; i < LED_PATTERNS_QTY; i++) {
            if (led_patterns[i].event == state_led_dta.event) {
                current_pattern = &led_patterns[i];
                break;
            }
        }

        // Si encontramos un patrón registrado
        if (current_pattern != NULL) {
            if (current_pattern->blinking_period == 0) {
                // Modo Fijo
                apply_led_colors(current_pattern->red, current_pattern->green, current_pattern->blue);
                state_led_dta.state = ST_STATE_LED_ON;
            } else {
                // Modo Parpadeante (arrancamos encendidos)
                apply_led_colors(current_pattern->red, current_pattern->green, current_pattern->blue);
                state_led_dta.tick = current_tick;
                state_led_dta.state = ST_STATE_LED_BLINK_ON;
            }
        } else {
            // Evento no reconocido o apagado
            apply_led_colors(0, 0, 0);
            state_led_dta.state = ST_STATE_LED_IDLE;
        }
        return;
    }

    // --- 2. TRANSICIONES DE TIEMPO (Blink Machine) ---
    switch (state_led_dta.state) {
        case ST_STATE_LED_IDLE:
        case ST_STATE_LED_ON:
            break; // No hay que controlar tiempo

        case ST_STATE_LED_BLINK_ON:
            if ((current_tick - state_led_dta.tick) >= current_pattern->blinking_period) {
                apply_led_colors(0, 0, 0); // Lo apagamos
                state_led_dta.tick = current_tick;
                state_led_dta.state = ST_STATE_LED_BLINK_OFF;
            }
            break;

        case ST_STATE_LED_BLINK_OFF:
            if ((current_tick - state_led_dta.tick) >= current_pattern->blinking_period) {
                apply_led_colors(current_pattern->red, current_pattern->green, current_pattern->blue); // Lo prendemos de nuevo
                state_led_dta.tick = current_tick;
                state_led_dta.state = ST_STATE_LED_BLINK_ON;
            }
            break;
    }
}

static void apply_led_colors(uint16_t pwm_red, uint16_t pwm_green, uint16_t pwm_blue) {
    __HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_RED, pwm_red);
    __HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_GREEN, pwm_green);
    __HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_BLUE, pwm_blue);
}
