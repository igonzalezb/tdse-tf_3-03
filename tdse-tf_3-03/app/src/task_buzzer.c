#include "main.h"
#include "app.h"
#include "task_buzzer.h"
#include "logger.h"
#include "task_menu_interface.h" // Acceso la enumeración task_menu_sys_t

// Tiempos en milisegundos
#define BUZZER_PULSE_MS        80   // Duración del beep corto de confirmación
#define BUZZER_INTERMITTENT_MS 300  // Velocidad de la alarma de falla (ON/OFF)

// Estados internos del buzzer
typedef enum buzzer_internal_st {
    B_STATE_IDLE,
    B_STATE_PULSE_ON,
    B_STATE_INT_ON,
    B_STATE_INT_OFF
} buzzer_internal_st_t;

// Variables estáticas
static buzzer_internal_st_t internal_state = B_STATE_IDLE;
static uint32_t start_tick = 0;
static task_menu_sys_t last_system = (task_menu_sys_t)-1;

// Prototipos de funciones
static void buzzer_on(void);
static void buzzer_off(void);


// Funciones auxiliares
static void buzzer_on(void) {
    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET);
}

static void buzzer_off(void) {
    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
}

void task_buzzer_init(void *parameters) {
    buzzer_off(); // Arranque apagado
}

void task_buzzer_update(void *parameters) {
    uint32_t current_tick = HAL_GetTick();
    task_menu_sys_t current_system = shared_data.active_system;

    // 1. Evaluar cambios de sistema
    if (current_system != last_system) {
        last_system = current_system;

        if (current_system == SYS_FAILURE) {
            buzzer_on();
            start_tick = current_tick;
            internal_state = B_STATE_INT_ON;
        } else {
            // Genera un pulso corto de confirmación sonora al entrar a Normal, Setup o Test
            buzzer_on();
            start_tick = current_tick;
            internal_state = B_STATE_PULSE_ON;
        }
    }

    // 2. Máquina de estados temporal y lógica de hardware
    switch (internal_state) {
        case B_STATE_IDLE:
            // Si no hay un pulso activo y estamos en modo TEST, obedecemos a la estructura unificada
        	if (current_system == SYS_TEST && shared_data.active_test == TEST_BUZZER) {
				buzzer_on();
			} else {
				buzzer_off();
			}
			break;

        case B_STATE_PULSE_ON:
            if ((current_tick - start_tick) >= BUZZER_PULSE_MS) {
                buzzer_off();
                internal_state = B_STATE_IDLE;
            }
            break;

        case B_STATE_INT_ON:
            // Resguardo por si el sistema sale de SYS_FAILURE inesperadamente
            if (current_system != SYS_FAILURE) {
                buzzer_off();
                internal_state = B_STATE_IDLE;
                break;
            }
            if ((current_tick - start_tick) >= BUZZER_INTERMITTENT_MS) {
                buzzer_off();
                start_tick = current_tick;
                internal_state = B_STATE_INT_OFF;
            }
            break;

        case B_STATE_INT_OFF:
            if (current_system != SYS_FAILURE) {
                internal_state = B_STATE_IDLE;
                break;
            }
            if ((current_tick - start_tick) >= BUZZER_INTERMITTENT_MS) {
                buzzer_on();
                start_tick = current_tick;
                internal_state = B_STATE_INT_ON;
            }
            break;
    }
}
