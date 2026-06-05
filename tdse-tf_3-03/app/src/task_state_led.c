#include "main.h"
#include "app.h"
#include "task_state_led.h"

#define FAST 150
#define SLOW 500

// Definición de canales
#define CHANNEL_LED_RED   TIM_CHANNEL_4
#define CHANNEL_LED_GREEN TIM_CHANNEL_3
#define CHANNEL_LED_BLUE  TIM_CHANNEL_1
#define TIMER_STATE_LED htim4

/*
 * LED_R TIM4_CH4 	PB9
 * LED_G TIM4_CH3	PB8
 * LED_B TIM4_CH1	PB6
 */

extern TIM_HandleTypeDef TIMER_STATE_LED;

void task_state_led_init(void *parameters) {
    // Inicializar PWM
    HAL_TIM_PWM_Start(&TIMER_STATE_LED, CHANNEL_LED_RED);
    HAL_TIM_PWM_Start(&TIMER_STATE_LED, CHANNEL_LED_GREEN);
    HAL_TIM_PWM_Start(&TIMER_STATE_LED, CHANNEL_LED_BLUE);

    // Forzar el Hardware a 0 (Apagar inmediatamente el LED por si acaso)
	__HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_RED, 0);
	__HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_GREEN, 0);
	__HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_BLUE, 0);

	// Inicializar el estado lógico del software (Datos Compartidos)
	shared_data.pwm_state_led_red = 0;
	shared_data.pwm_state_led_green = 0;
	shared_data.pwm_state_led_blue = 0;
	shared_data.blinking_rate = STATE_LED_NO_BLINK;

	// Flag de cambios:
	shared_data.state_led_data_changed = false;
}

void task_state_led_update(void *parameters) {
    static uint32_t last_blink_tick = 0;
    static bool led_toggle_state = true;

    uint32_t current_tick = HAL_GetTick();
    uint32_t blink_period = 0;

    bool update_hardware_needed = false;

    if (shared_data.state_led_data_changed) {
		update_hardware_needed = true;
		shared_data.state_led_data_changed = false;
    }

    switch (shared_data.blinking_rate) {
        case STATE_LED_SLOW_BLINK: blink_period = SLOW; break;
        case STATE_LED_FAST_BLINK: blink_period = FAST; break;
        case STATE_LED_NO_BLINK:
        default:
        	blink_period = 0;
			if (!led_toggle_state) {
				led_toggle_state = true;
				update_hardware_needed = true;
			}
			break;
    }

    if (blink_period > 0 && (current_tick - last_blink_tick) >= blink_period) {
		last_blink_tick = current_tick;
		led_toggle_state = !led_toggle_state; // Alternamos el estado lógico (ON/OFF)
		update_hardware_needed = true;        // Como cambió el estado lógico, actualizamos hardware
	}

    if (update_hardware_needed) {
		if (led_toggle_state) {
			// Fase ON: Escribir la mezcla de colores configurada
			__HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_RED, shared_data.pwm_state_led_red);
			__HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_GREEN, shared_data.pwm_state_led_green);
			__HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_BLUE, shared_data.pwm_state_led_blue);
		} else {
			// Fase OFF: Forzar a 0 todos los canales (Bug corregido aquí)
			__HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_RED, 0);
			__HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_GREEN, 0);
			__HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_BLUE, 0);
		}
	}
}
