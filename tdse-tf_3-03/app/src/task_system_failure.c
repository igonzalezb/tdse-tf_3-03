
/********************** inclusions *******************************************/
#include "app.h"
#include "main.h"
#include "task_system_failure.h"
#include "task_menu_interface.h"
#include "task_actuator_attribute.h"
#include "task_pump.h"
#include "task_led_strip.h"
#include "logger.h"

// Comentar la linea de abajo para desactivar modo falla
#define ENABLE_FAILURE


/********************** defines *******************************************/
#define MAX_OVERCURRENT_FAILURES 2
// periodos en los que se anula el sensado de fallas para evitar falsos positivos
#define STARTUP_PERIOD_MS 150
#define ACTUATOR_PERIOD_MS 300

/** Variables estáticas **/
static volatile bool active_faults[FAULT_QTY];		// Arreglo estático de las fallas activas
static bool lock_system = false;      				// Flag: se debe bloquear el sistema por repetidos excesos de corriente?
static uint8_t pump_overcurrent_failures = 0; 		// Contador de fallas de sobre corriente de bomba
static uint8_t led_strip_overcurrent_failures = 0; 	// Contador de fallas de sobre corriente de tira led



// Nombres que se mostrarán en la 2da línea de la pantalla (Máximo 16 caracteres)
const char* fault_names[FAULT_QTY] = {
    "Corriente  Bomba",
    "Bomba Desconect.",
    "CorrienteTiraLED",
    "Ilum. Desconect.",
    "Temperatura Alta",
	"Temperatura Baja",
    "DHT22 No Resp.",
    "Nivel Agua Bajo",
    "Sens. Nivel Agua",
    "Sens. Nivel Luz",
    "Sens.HumedadTierr",
	"Driver Bomba",
	"Driver Tira LED"
};

void task_system_failure_init(void *parameters){
	for(int i=0; i<FAULT_QTY; i++){
		active_faults[i] = false;
	}
	lock_system = false;
	pump_overcurrent_failures = 0;
	led_strip_overcurrent_failures = 0;
}

void task_system_failure_update(void *parameters) {
	#ifdef ENABLE_FAILURE
	static uint32_t init_tick = 0;
	static bool system_ready = false;

	// === Temporizadores de estado para actuadores ===
	static uint32_t pump_on_tick = 0;
	static uint32_t pump_off_tick = 0;
	static uint32_t led_strip_on_tick = 0;
	static uint32_t led_strip_off_tick = 0;

	if (!system_ready) {
			if (init_tick == 0) {
			init_tick = HAL_GetTick();
		}

		// Si aún no pasó el tiempo de gracia, salimos de la función inmediatamente
		if ((HAL_GetTick() - init_tick) < STARTUP_PERIOD_MS) {
			return;
		}

		// El tiempo de gracia finalizó, marcamos el sistema como listo
		system_ready = true;
		LOGGER_INFO("Iniciando monitoreo de fallas.");
	}


	// ======================     BOMBA     ==========================
	if (get_pump_state() == ST_PUMP_ON) {
			pump_off_tick = 0; // Se encendió, cancelamos el timer de apagado

			if (pump_on_tick == 0) pump_on_tick = HAL_GetTick(); // Registramos cuándo arrancó

			if ((HAL_GetTick() - pump_on_tick) > ACTUATOR_PERIOD_MS) {
				if(shared_data.pump_current_ma > MAX_PUMP_CURRENT){
					task_system_failure_report(FAULT_PUMP_OVERCURRENT);
				}
				else if(shared_data.pump_current_ma < MIN_PUMP_CURRENT){
					task_system_failure_report(FAULT_PUMP_OPEN);
				}
			}
		}
		else if (get_pump_state() == ST_PUMP_IDLE) {
			pump_on_tick = 0; // Se apagó, cancelamos el timer de encendido

			if (pump_off_tick == 0) pump_off_tick = HAL_GetTick(); // Registramos cuándo se apagó

			if ((HAL_GetTick() - pump_off_tick) > ACTUATOR_PERIOD_MS) {
				// Si pasó el tiempo de gracia y sigue habiendo corriente, el driver falló
				if(shared_data.pump_current_ma > MIN_PUMP_CURRENT){
					task_system_failure_report(FAULT_PUMP_DRIVER);
				}
			}
		} else {
			// Para estados intermedios (ej. ST_PUMP_RAMP_UP / RAMP_DOWN) no se evaluan fallos
			pump_on_tick = 0;
			pump_off_tick = 0;
		}

	// ======================     Tira LED     ==========================
		if (get_led_strip_state() == ST_LED_STRIP_ON) {
			led_strip_off_tick = 0;

			if (led_strip_on_tick == 0) led_strip_on_tick = HAL_GetTick();

			if ((HAL_GetTick() - led_strip_on_tick) > ACTUATOR_PERIOD_MS) {
				if(shared_data.led_current_ma > MAX_LED_STRIP_CURRENT){
					task_system_failure_report(FAULT_LED_STRIP_OVERCURRENT);
				}
				else if(shared_data.led_current_ma < MIN_LED_STRIP_CURRENT){
					task_system_failure_report(FAULT_LED_STRIP_OPEN);
				}
			}
		}
		else if (get_led_strip_state() == ST_LED_STRIP_OFF) {
			led_strip_on_tick = 0;

			if (led_strip_off_tick == 0) led_strip_off_tick = HAL_GetTick();

			if ((HAL_GetTick() - led_strip_off_tick) > ACTUATOR_PERIOD_MS) {
				if(shared_data.led_current_ma > MIN_LED_STRIP_CURRENT){
					task_system_failure_report(FAULT_LED_STRIP_DRIVER);
				}
			}
		}

	// ======================     DHT 22     ==========================
	// pruebo directamente sin pausa
	if(shared_data.dht22_temperature > MAX_TEMPERATURE){
		task_system_failure_report(FAULT_HIGH_TEMPERATURE);
	}

	if(shared_data.dht22_temperature < MIN_TEMPERATURE){
		task_system_failure_report(FAULT_LOW_TEMPERATURE);
	}

	if(shared_data.dht22_error){
		task_system_failure_report(FAULT_DHT22_NO_RESPONSE);
	}
//}
	if(shared_data.water_level_percent < shared_data.config_values[CONFIG_WATER_LEVEL]){
		task_system_failure_report(FAULT_WATER_LEVEL_LOW);
	}

	if(shared_data.water_level_percent <= 0){
		task_system_failure_report(FAULT_WATER_LEVEL_ERROR);
	}
	if(shared_data.light_percent <= 0){
		task_system_failure_report(FAULT_LIGHT_LEVEL_ERROR);
	}
	if(shared_data.humidity_percent <= 0){
		task_system_failure_report(FAULT_HUMIDITY_LEVEL_ERROR);
	}
	#endif
}

bool task_system_failure_is_locked(void){
	return lock_system;
}

void task_system_failure_report(system_failure_type failure) {
	if (failure >= FAULT_QTY) return;

	if (active_faults[failure] == false) {
		active_faults[failure] = true;

		put_event_task_menu(EV_SYS_FAILURE);

		// Contadores de sobrecorriente. Si llega a 2, bloquea el sistema.
		if (failure == FAULT_PUMP_OVERCURRENT) {
			pump_overcurrent_failures++;
			if (pump_overcurrent_failures >= MAX_OVERCURRENT_FAILURES) {
				lock_system = true;
			}
		}
		else if (failure == FAULT_LED_STRIP_OVERCURRENT) {
			led_strip_overcurrent_failures++;
			if (led_strip_overcurrent_failures >= MAX_OVERCURRENT_FAILURES) {
				lock_system = true;
			}
		}
	}


}

bool task_system_failure_can_restore(void) {
    if (lock_system) return false;	// El sistema está bloqueado (por límite de sobrecorrientes).

    for (int i = 0; i < FAULT_QTY; i++) {
    	// Solo se evalúan las activas
        if (active_faults[i] == true) {

            switch(i) {
                // Verificamos si el sensor ya volvió a valores seguros
            	case FAULT_PUMP_OVERCURRENT:
					if (shared_data.pump_current_ma >= MAX_PUMP_CURRENT) return false;
					break;

            	case FAULT_PUMP_OPEN: break;

            	case FAULT_LED_STRIP_OVERCURRENT:
					if (shared_data.led_current_ma >= MAX_LED_STRIP_CURRENT) return false;
					break;

				case FAULT_LED_STRIP_OPEN: break;

                case FAULT_HIGH_TEMPERATURE:
                    if (shared_data.dht22_temperature >= MAX_TEMPERATURE) return false;
                    break;

                case FAULT_LOW_TEMPERATURE:
                    if (shared_data.dht22_temperature <= MIN_TEMPERATURE) return false;
                    break;

                case FAULT_DHT22_NO_RESPONSE:
                    if (shared_data.dht22_error != 0) return false;
                    break;

                case FAULT_WATER_LEVEL_LOW:
                    if (shared_data.water_level_percent <= shared_data.config_values[CONFIG_WATER_LEVEL]) return false;
                    break;

                case FAULT_WATER_LEVEL_ERROR:
                    if (shared_data.water_level_percent == 0) return false;
                    break;

                case FAULT_LIGHT_LEVEL_ERROR:
                    if (shared_data.light_percent == 0) return false;
                    break;

                case FAULT_HUMIDITY_LEVEL_ERROR:
                    if (shared_data.humidity_percent == 0) return false;
                    break;

                case FAULT_PUMP_DRIVER:
					if (shared_data.pump_current_ma >=  MIN_PUMP_CURRENT) return false;
					break;

				case FAULT_LED_STRIP_DRIVER:
					if (shared_data.led_current_ma >=  MIN_LED_STRIP_CURRENT) return false;
					break;

                default: break;
            }
        }
    }

    return true;
}


const char* task_system_failure_get_name(system_failure_type failure) {
    if (failure >= FAULT_QTY) return "Desconocida";
    return fault_names[failure];
}

// Evalúa si el índice actual es válido. Si no lo es, devuelve el correcto.
system_failure_type task_system_failure_get_valid_fault(system_failure_type failure) {
    bool ready_to_restore = task_system_failure_can_restore();
    // failure pide restaurar y se puede restaurar:
    if (failure == FAULT_RESTORE && ready_to_restore) return failure;
    // failure pertenece a la lista y es una falla activa:
    if (failure < FAULT_QTY && active_faults[failure]) return failure;
    // Si no es ninguno: se busca la primera falla activa
    for(int i = 0; i < FAULT_QTY; i++) {
        if (active_faults[i]) return i;
    }
    // No hay fallas activas -> Restaurar
    return FAULT_RESTORE;
}

void task_system_failure_clear_all(void) {
    for(int i = 0; i < FAULT_QTY; i++) {
        active_faults[i] = false;
    }
}

system_failure_type task_system_failure_get_next(system_failure_type failure){
    bool can_restore = task_system_failure_can_restore();
    system_failure_type next = failure;

    for(int i = 0; i <= FAULT_QTY + 1; i++) {
    	if (next == FAULT_RESTORE) {
			next = 0; // Si se llega al ultimo item, vuelve a empezar
		} else {
			next++;
			if (next == FAULT_QTY) {
				next = can_restore ? FAULT_RESTORE : 0;
			}
		}
		if (next == FAULT_RESTORE && can_restore) return FAULT_RESTORE;
		if (next < FAULT_QTY && active_faults[next]) return next;
    }

    return failure;
}

system_failure_type task_system_failure_get_prev(system_failure_type failure){
	bool can_restore = task_system_failure_can_restore();
	system_failure_type prev = failure;

	for(int i = 0; i <= FAULT_QTY + 1; i++) {
		if (prev == 0) {
			prev = can_restore ? FAULT_RESTORE : (FAULT_QTY - 1);
		} else if (prev == FAULT_RESTORE) {
			// Desde RESTORE, salta a la última falla válida
			prev = FAULT_QTY - 1;
		} else {
			prev--;
		}
		if (prev == FAULT_RESTORE && can_restore) return FAULT_RESTORE;
		if (prev < FAULT_QTY && active_faults[prev]) return prev;
	}
	return failure;
}
