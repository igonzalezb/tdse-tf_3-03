#include "main.h"
#include "task_actuator_interface.h"
#include "task_pump.h"

#define CHANNEL_PUMP   TIM_CHANNEL_4
#define TIMER_PUMP     htim1

extern TIM_HandleTypeDef TIMER_PUMP;

// Configuración del Soft-Starter
#define PUMP_PWM_MAX       3199
#define PUMP_PWM_MIN       480

// La rampa tardará: (MAX / STEP) * DELAY_MS.
// En este caso, debería tomar 650 ms pasar de 0% a 100%
#define PUMP_RAMP_STEP     10    // Incremento/Decremento de PWM por ciclo
#define PUMP_RAMP_DELAY_MS 1   // Tiempo de espera entre pasos


// ESTRUCTURAS INTERNAS
static task_actuator_dta_t pump_dta;
static uint16_t current_duty = 0; // Memoria del nivel actual de la rampa

// prototipos de funciones privadas
static void task_pump_statechart(void);
static void apply_pump_pwm(uint16_t);

void task_pump_init(void *parameters) {
    HAL_TIM_PWM_Start(&TIMER_PUMP, CHANNEL_PUMP);

    current_duty = 0;
    apply_pump_pwm(current_duty);

    pump_dta.state = ST_PUMP_IDLE;
    pump_dta.event_pending = false;
    pump_dta.event = EV_PUMP_OFF;

    init_queue_event_task_actuator(ID_ACT_PUMP);
}

void task_pump_update(void *parameters) {
    if (any_event_task_actuator(ID_ACT_PUMP)) {
        pump_dta.event = get_event_task_actuator(ID_ACT_PUMP);
        pump_dta.event_pending = true;
    }

    task_pump_statechart();
}

static void task_pump_statechart(void) {
    uint32_t current_tick = HAL_GetTick();

    // --- 1. PROCESAR NUEVOS EVENTOS ---
    if (pump_dta.event_pending) {
        pump_dta.event_pending = false;

        switch(pump_dta.event) {
            case EV_PUMP_ON:
                // Si estaba apagada o apagándose, empezamos a subir
                if (pump_dta.state == ST_PUMP_IDLE || pump_dta.state == ST_PUMP_RAMP_DOWN) {
                    pump_dta.state = ST_PUMP_RAMP_UP;
                    pump_dta.tick = current_tick;
                }
                if (current_duty == 0) {
					current_duty = PUMP_PWM_MIN;
					apply_pump_pwm(current_duty);
				}
                break;

            case EV_PUMP_OFF:
                // Si estaba prendida o prendiéndose, empezamos a bajar
                if (pump_dta.state == ST_PUMP_ON || pump_dta.state == ST_PUMP_RAMP_UP) {
                    pump_dta.state = ST_PUMP_RAMP_DOWN;
                    pump_dta.tick = current_tick;
                }
                break;

            default:
                break;
        }
    }

    // --- 2. MÁQUINA DE ESTADOS (Control de rampa) ---
    switch (pump_dta.state) {
        case ST_PUMP_IDLE:
        case ST_PUMP_ON:
            // Estáticos. El PWM ya está en su valor objetivo (0 o MAX).
            break;

        case ST_PUMP_RAMP_UP:
            if ((current_tick - pump_dta.tick) >= PUMP_RAMP_DELAY_MS) {
                pump_dta.tick = current_tick;

                if (current_duty < PUMP_PWM_MAX) {
                    current_duty += PUMP_RAMP_STEP;
                    if (current_duty > PUMP_PWM_MAX) current_duty = PUMP_PWM_MAX; // Límite seguro

                    apply_pump_pwm(current_duty);
                } else {
                    // Llegamos al máximo
                    pump_dta.state = ST_PUMP_ON;
                }
            }
            break;

        case ST_PUMP_RAMP_DOWN:
            if ((current_tick - pump_dta.tick) >= PUMP_RAMP_DELAY_MS) {
                pump_dta.tick = current_tick;

                if (current_duty > PUMP_PWM_MIN) {
                    // Restamos cuidando de no hacer un underflow
                    if (current_duty > PUMP_RAMP_STEP) {
                        current_duty -= PUMP_RAMP_STEP;
                    } else {
                        current_duty = PUMP_PWM_MIN;
                    }

                    apply_pump_pwm(current_duty);
                } else {
                    current_duty = 0;
					apply_pump_pwm(current_duty);
					pump_dta.state = ST_PUMP_IDLE;
                }
            }
            break;
    }
}

static void apply_pump_pwm(uint16_t pump_pwm_duty) {
    __HAL_TIM_SET_COMPARE(&TIMER_PUMP, CHANNEL_PUMP, pump_pwm_duty);
}

task_pump_st_t get_pump_state(void) {
    return (task_pump_st_t)pump_dta.state;
}
