#include "main.h"
#include "app.h"
#include "task_buzzer.h"
#include "logger.h"

// Tiempos en milisegundos
#define BUZZER_PULSE_MS        80  // Duración del beep corto de confirmación
#define BUZZER_INTERMITTENT_MS 300  // Velocidad de la alarma de falla (ON/OFF)

#ifndef OFF
#define OFF    0
#endif
#ifndef ON
#define ON     ( !OFF )
#endif

/*
 * BUZZER
 * Buzzer_Pin GPIO_PIN_13
 * Buzzer_GPIO_Port GPIOB
 */

// Estados internos del buzzer
typedef enum buzzer_internal_st{
    B_STATE_IDLE,
    B_STATE_PULSE_ON,
    B_STATE_INT_ON,
    B_STATE_INT_OFF
} buzzer_internal_st_t;

// Funciones auxiliares para escribir menos
static void buzzer_on(void) {
    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET);
}

static void buzzer_off(void) {
    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
}


void task_buzzer_init(void *parameters) {
    buzzer_off(); // Arranque apagado
    shared_data.buzzer_mode = BUZZER_MODE_OFF;
    shared_data.buzzer_mode_changed = false;
}

void task_buzzer_update(void *parameters) {
    static buzzer_internal_st_t internal_state = B_STATE_IDLE;
    static uint32_t start_tick = 0;
    uint32_t current_tick = HAL_GetTick();

    // 1. Evaluar si hubo un cambio de orden desde el menú
    if (shared_data.buzzer_mode_changed) {
        shared_data.buzzer_mode_changed = false;

        switch (shared_data.buzzer_mode) {
            case BUZZER_MODE_PULSE:
                buzzer_on();
                start_tick = current_tick;
                internal_state = B_STATE_PULSE_ON;
                break;
            case BUZZER_MODE_INTERMITTENT:
                buzzer_on();
                start_tick = current_tick;
                internal_state = B_STATE_INT_ON;
                break;
            case BUZZER_MODE_OFF:
            default:
                buzzer_off();
                internal_state = B_STATE_IDLE;
                break;
        }
    }

    // 2. Máquina de estados temporal
    switch (internal_state) {
        case B_STATE_IDLE:
            break;

        case B_STATE_PULSE_ON:
            if ((current_tick - start_tick) >= BUZZER_PULSE_MS) {
                buzzer_off();
                internal_state = B_STATE_IDLE;
                shared_data.buzzer_mode = BUZZER_MODE_OFF; // Restaurar lógica
            }
            break;

        case B_STATE_INT_ON:
            if ((current_tick - start_tick) >= BUZZER_INTERMITTENT_MS) {
                buzzer_off();
                start_tick = current_tick;
                internal_state = B_STATE_INT_OFF;
            }
            break;

        case B_STATE_INT_OFF:
            if ((current_tick - start_tick) >= BUZZER_INTERMITTENT_MS) {
                buzzer_on();
                start_tick = current_tick;
                internal_state = B_STATE_INT_ON;
            }
            break;
    }
}
