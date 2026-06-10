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

/*
// Tiempos de parpadeo
typedef enum blinking_period_ms {
	STATE_LED_NO_BLINK   = 0,
	STATE_LED_FAST_BLINK = 150,
	STATE_LED_SLOW_BLINK = 500
} blinking_period_ms_t;

typedef enum state_led_internal_st{
    STATE_LED_ST_IDLE,
    STATE_LED_ST_ON,
    STATE_LED_ST_SLOW_ON,
    STATE_LED_ST_SLOW_OFF,
    STATE_LED_ST_FAST_ON,
    STATE_LED_ST_FAST_OFF
} state_led_internal_st_t;


static task_actuator_dta_t state_led_dta;

// ==========================================================
// Lógica de Hardware: Aplicar Colores (Tu lógica original)
// ==========================================================
static void apply_led_colors(bool turn_on) {
    if (!turn_on) {
        // Apagar los 3 canales
        __HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_RED, 0);
        __HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_GREEN, 0);
        __HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_BLUE, 0);
        return;
    }

    // Calcular color según shared_data (Tal cual tu código original)
    uint16_t pwm_state_led_red = 0, pwm_state_led_green = 0, pwm_state_led_blue = 0;
    task_menu_sys_t current_system = shared_data.active_system;
    uint8_t current_test = shared_data.active_test;

    if (current_system == SYS_TEST && current_test == TEST_STATE_LED) {
        pwm_state_led_red = 100; pwm_state_led_green = 100; pwm_state_led_blue = 100;
    } else {
        switch (current_system) {
            case SYS_NORMAL:  pwm_state_led_green = 100; break;
            case SYS_SETUP:   pwm_state_led_blue = 75; break;
            case SYS_FAILURE: pwm_state_led_red = 100; break;
            case SYS_TEST:    pwm_state_led_red = 50; pwm_state_led_blue = 50; break;
            default: break;
        }
    }

    // Fase ON: Escribir la mezcla de colores
    __HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_RED, pwm_state_led_red);
    __HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_GREEN, pwm_state_led_green);
    __HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_BLUE, pwm_state_led_blue);
}

// ==========================================================
// FUNCIONES PÚBLICAS Y STATECHART
// ==========================================================
void task_state_led_init(void *parameters) {
    // Inicializar PWM
    HAL_TIM_PWM_Start(&TIMER_STATE_LED, CHANNEL_LED_RED);
    HAL_TIM_PWM_Start(&TIMER_STATE_LED, CHANNEL_LED_GREEN);
    HAL_TIM_PWM_Start(&TIMER_STATE_LED, CHANNEL_LED_BLUE);

    apply_led_colors(false); // Arranca apagado por seguridad

    // Configurar statechart
    state_led_dta.state = STATE_LED_ST_IDLE;
    state_led_dta.event = EV_STATE_LED_OFF;
    state_led_dta.event_pending = false;
}

static void task_state_led_statechart(void) {
    uint32_t current_tick = HAL_GetTick();

    // --- 1. PROCESAR NUEVOS EVENTOS ---
    if (state_led_dta.event_pending) {
        state_led_dta.event_pending = false;

        switch (state_led_dta.event) {
            case EV_STATE_LED_ON:
                apply_led_colors(true);
                state_led_dta.state = STATE_LED_ST_ON;
                break;

            case EV_STATE_LED_SLOW_BLINKING:
                apply_led_colors(true);
                state_led_dta.tick = current_tick;
                state_led_dta.state = STATE_LED_ST_SLOW_ON;
                break;

            case EV_STATE_LED_FAST_BLINKING:
                apply_led_colors(true);
                state_led_dta.tick = current_tick;
                state_led_dta.state = STATE_LED_ST_FAST_ON;
                break;

            case EV_STATE_LED_OFF:
            default:
                apply_led_colors(false);
                state_led_dta.state = STATE_LED_ST_IDLE;
                break;
        }
        return;
    }

    // --- 2. TRANSICIONES DE TIEMPO (Blink Machine) ---
    switch (state_led_dta.state) {
        case STATE_LED_ST_IDLE:
            break;

        case STATE_LED_ST_ON:
            // Refresca el color constantemente. Si shared_data cambia de Normal (Verde)
            // a Error (Rojo) mientras está encendido, el color se actualizará solo.
            apply_led_colors(true);
            break;

        case STATE_LED_ST_SLOW_ON:
            apply_led_colors(true);
            if ((current_tick - state_led_dta.tick) >= STATE_LED_SLOW_BLINK) {
                apply_led_colors(false);
                state_led_dta.tick = current_tick;
                state_led_dta.state = STATE_LED_ST_SLOW_OFF;
            }
            break;

        case STATE_LED_ST_SLOW_OFF:
            if ((current_tick - state_led_dta.tick) >= STATE_LED_SLOW_BLINK) {
                apply_led_colors(true);
                state_led_dta.tick = current_tick;
                state_led_dta.state = STATE_LED_ST_SLOW_ON;
            }
            break;

        case STATE_LED_ST_FAST_ON:
            apply_led_colors(true);
            if ((current_tick - state_led_dta.tick) >= STATE_LED_FAST_BLINK) {
                apply_led_colors(false);
                state_led_dta.tick = current_tick;
                state_led_dta.state = STATE_LED_ST_FAST_OFF;
            }
            break;

        case STATE_LED_ST_FAST_OFF:
            if ((current_tick - state_led_dta.tick) >= STATE_LED_FAST_BLINK) {
                apply_led_colors(true);
                state_led_dta.tick = current_tick;
                state_led_dta.state = STATE_LED_ST_FAST_ON;
            }
            break;
    }
}

void task_state_led_update(void *parameters) {
    // 1. Revisar si hay ordenes en la cola de actuadores
    if (any_event_task_actuator(ID_ACT_STATE_LED)) {
        state_led_dta.event = (task_state_led_ev_t) get_event_task_actuator(ID_ACT_STATE_LED);
        state_led_dta.event_pending = true;
    }

    // 2. Ejecutar la máquina de estados
    task_state_led_statechart();
}
*/
