#include "main.h"
#include "app.h"
#include "task_state_led.h"
#include "task_actuator_interface.h"

// Definición de canales
#define CHANNEL_LED_RED   TIM_CHANNEL_4
#define CHANNEL_LED_GREEN TIM_CHANNEL_3
#define CHANNEL_LED_BLUE  TIM_CHANNEL_1
#define TIMER_STATE_LED htim4

extern TIM_HandleTypeDef TIMER_STATE_LED;

// Variables estáticas
static task_actuator_dta_t state_led_dta;
static const state_led_pattern_t *current_pattern = NULL;

// Funciones internas
static void apply_led_colors(uint16_t pwm_red, uint16_t pwm_green, uint16_t pwm_blue);
static void task_state_led_statechart(void);

// Constantes
uint16_t const LED_PATTERNS_QTY = (sizeof(led_patterns) / sizeof(led_patterns[0]));


static void apply_led_colors(uint16_t pwm_red, uint16_t pwm_green, uint16_t pwm_blue) {
    __HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_RED, pwm_red);
    __HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_GREEN, pwm_green);
    __HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_BLUE, pwm_blue);
}


void task_state_led_init(void *parameters) {
    HAL_TIM_PWM_Start(&TIMER_STATE_LED, CHANNEL_LED_RED);
    HAL_TIM_PWM_Start(&TIMER_STATE_LED, CHANNEL_LED_GREEN);
    HAL_TIM_PWM_Start(&TIMER_STATE_LED, CHANNEL_LED_BLUE);

    apply_led_colors(0, 0, 0);

    state_led_dta.state = ST_STATE_LED_IDLE;
    state_led_dta.event_pending = false;
    current_pattern = NULL;
}

static void task_state_led_statechart(void) {
    uint32_t current_tick = HAL_GetTick();

    if (state_led_dta.event_pending) {
        state_led_dta.event_pending = false;

        // Buscamos si el evento recibido existe en nuestro diccionario de patrones
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

void task_state_led_update(void *parameters) {
    if (any_event_task_actuator(ID_ACT_STATE_LED)) {
        state_led_dta.event = get_event_task_actuator(ID_ACT_STATE_LED);
        state_led_dta.event_pending = true;
    }

    task_state_led_statechart();
}
