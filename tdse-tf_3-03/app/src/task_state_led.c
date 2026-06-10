#include "main.h"
#include "app.h"
#include "task_state_led.h"
#include "task_menu_interface.h"

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

// variables estáticas
static uint32_t last_blink_tick = 0;
static bool led_toggle_state= true;
static task_menu_sys_t last_system = (task_menu_sys_t)-1;
static uint8_t last_test = 255;

// Definicion de tiempos de parpadeo
typedef enum blinking_period_ms {
	STATE_LED_NO_BLINK   = 0,	// No son 0 ms, significa que no parpadea el LED
	STATE_LED_FAST_BLINK = 150,
	STATE_LED_SLOW_BLINK = 500
} blinking_period_ms_t;

void task_state_led_init(void *parameters) {
    // Inicializar PWM
    HAL_TIM_PWM_Start(&TIMER_STATE_LED, CHANNEL_LED_RED);
    HAL_TIM_PWM_Start(&TIMER_STATE_LED, CHANNEL_LED_GREEN);
    HAL_TIM_PWM_Start(&TIMER_STATE_LED, CHANNEL_LED_BLUE);

    // Forzar el Hardware a 0 (Apagar inmediatamente el LED por si acaso)
	__HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_RED, 0);
	__HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_GREEN, 0);
	__HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_BLUE, 0);

}

void task_state_led_update(void *parameters) {
    uint32_t current_tick = HAL_GetTick();
    bool update_hardware_needed = false;

    // Variables locales para calcular el comportamiento deseado
    uint16_t pwm_state_led_red = 0, pwm_state_led_green = 0, pwm_state_led_blue = 0;
    blinking_period_ms_t blink_period = STATE_LED_NO_BLINK;

    task_menu_sys_t current_system = shared_data.active_system;
    uint8_t current_test = shared_data.active_test;

    // 1. Determinar color y parpadeo de forma autónoma
    if (current_system == SYS_TEST && current_test == TEST_STATE_LED) {
        // --- MODO TEST FÍSICO ---
        // Forzamos el encendido en color Blanco al 100% para verificar que anden los 3 diodos
        pwm_state_led_red = 100;
        pwm_state_led_green = 100;
        pwm_state_led_blue = 100;
        blink_period = STATE_LED_NO_BLINK;
    } else {
        // --- MODO INDICADOR DE SISTEMA NORMAL ---
        switch (current_system) {
            case SYS_NORMAL:
                pwm_state_led_red = 0; pwm_state_led_green = 100; pwm_state_led_blue = 0; // Verde
                blink_period = STATE_LED_NO_BLINK;
                break;
            case SYS_SETUP:
				pwm_state_led_red = 0; pwm_state_led_green = 0; pwm_state_led_blue = 75; // Azul
				blink_period = STATE_LED_SLOW_BLINK;
				break;
            case SYS_FAILURE:
                pwm_state_led_red = 100; pwm_state_led_green = 0; pwm_state_led_blue = 0; // Rojo
                blink_period = STATE_LED_FAST_BLINK;
                break;
            case SYS_TEST:
                // Si está en el menú TEST (pero probando otro actuador), muestra Magenta
                pwm_state_led_red = 50; pwm_state_led_green = 0; pwm_state_led_blue = 50;
                blink_period = STATE_LED_NO_BLINK;
                break;
            default:
                pwm_state_led_red = 0; pwm_state_led_green = 0; pwm_state_led_blue = 0; // Apagado por defecto
                blink_period = STATE_LED_NO_BLINK;
                break;
        }
    }

    // 2. Evaluar si hubo un cambio para refrescar inmediatamente
    if (current_system != last_system || current_test != last_test) {
        last_system = current_system;
        last_test = current_test;
        update_hardware_needed = true;
        led_toggle_state = true; // Reiniciamos el toggle para que arranque encendido en el nuevo estado
        last_blink_tick = current_tick;
    }

    // 3. Gestión del tiempo de parpadeo (Blink Machine)
    if (blink_period == 0) {
        // Si no debe parpadear, forzamos que la bandera se mantenga en TRUE (ON)
        if (!led_toggle_state) {
            led_toggle_state = true;
            update_hardware_needed = true;
        }
    } else {
        if ((current_tick - last_blink_tick) >= blink_period) {
            last_blink_tick = current_tick;
            led_toggle_state = !led_toggle_state; // Alternamos el estado lógico (ON/OFF)
            update_hardware_needed = true;
        }
    }

    // 4. Actualizar el Hardware PWM si algo cambió en la lógica o el toggle
    if (update_hardware_needed) {
        if (led_toggle_state) {
            // Fase ON: Escribir la mezcla de colores que calculamos arriba
            __HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_RED, pwm_state_led_red);
            __HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_GREEN, pwm_state_led_green);
            __HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_BLUE, pwm_state_led_blue);
        } else {
            // Fase OFF: Apagar los 3 canales temporalmente
            __HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_RED, 0);
            __HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_GREEN, 0);
            __HAL_TIM_SET_COMPARE(&TIMER_STATE_LED, CHANNEL_LED_BLUE, 0);
        }
    }
}
