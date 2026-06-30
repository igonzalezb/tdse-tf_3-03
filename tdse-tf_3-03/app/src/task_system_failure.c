
/********************** inclusions *******************************************/
#include "app.h"
#include "task_system_failure.h"

/********************** defines *******************************************/
#define MAX_OVERCURRENT_FAILURES 2
#define MAX_TEMP 35
#define MIN_TEMP 5
#define MAX_CURRENT 90
#define MIN_WATER_LEVEL 10

/** Variables estáticas **/
static bool active_faults[FAULT_QTY];				// Arreglo estático de las fallas activas
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
    "Sens.HumedadTierr"
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
	return;
}

bool task_system_failure_is_locked(void){
	return lock_system;
}

void task_system_failure_report(system_failure_type failure) {
	if (failure >= FAULT_QTY) return;

	if (active_faults[failure] == false) {
		active_faults[failure] = true;

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

        // Solo evaluamos si esa falla está registrada como activa
        if (active_faults[i] == true) {

            switch(i) {
                // Verificamos si el sensor ya volvió a valores seguros
                case FAULT_HIGH_TEMPERATURE:
                    if (shared_data.dht22_temperature >= MAX_TEMP) return false;
                    break;

                case FAULT_LOW_TEMPERATURE:
                    if (shared_data.dht22_temperature <= MIN_TEMP) return false;
                    break;

                case FAULT_DHT22_NO_RESPONSE:
                    if (shared_data.dht22_error != 0) return false;
                    break;

                case FAULT_WATER_LEVEL_LOW:
                    if (shared_data.water_level_percent <= MIN_WATER_LEVEL) return false;
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

                case FAULT_PUMP_OVERCURRENT:
                	if (shared_data.pump_current_percent >= MAX_CURRENT) return false;
                    break;

                case FAULT_PUMP_OPEN:
                case FAULT_LED_STRIP_OVERCURRENT:
                	if (shared_data.led_current_percent >= MAX_CURRENT) return false;
                	break;
                case FAULT_LED_STRIP_OPEN:
                    break;

                default:
                    break;
            }
        }
    }

    return true;
}


const char* task_system_failure_get_name(system_failure_type failure) {
    if (failure >= FAULT_QTY) return "Desconocida";
    return fault_names[failure];
}

// Evalúa si el índice actual es válido. Si no lo es, te devuelve el correcto.
system_failure_type task_system_failure_get_valid_fault(system_failure_type failure) {
    bool ready_to_restore = task_system_failure_can_restore();
    // FAULT_QTY se usa para mostrar la pantalla de restauracion por simplicidad de código
    if (failure == FAULT_QTY && ready_to_restore) return failure;

    // La falla sigue activa
    if (failure < FAULT_QTY && active_faults[failure]) return failure;

    // Se busca la primera falla activa
    for(int i = 0; i < FAULT_QTY; i++) {
        if (active_faults[i]) return i;
    }
    // No hay fallas activas -> Restaurar
    return FAULT_QTY;
}

void task_system_failure_clear_all(void) {
    for(int i = 0; i < FAULT_QTY; i++) {
        active_faults[i] = false;
    }
}

system_failure_type task_system_failure_get_next(system_failure_type current){
    bool can_restore = task_system_failure_can_restore();
    system_failure_type limit = can_restore ? (FAULT_QTY + 1) : FAULT_QTY;

    for(int i = 1; i <= limit; i++) {
        uint8_t next = (current + i) % limit;
        if (next == FAULT_QTY || active_faults[next]) {
            return next;
        }
    }
    return current; // Si no encuentra nada, se queda donde está
}

system_failure_type task_system_failure_get_prev(system_failure_type current){
    bool can_restore = task_system_failure_can_restore();
    system_failure_type limit = can_restore ? (FAULT_QTY + 1) : FAULT_QTY;

    for(int i = 1; i <= limit; i++) {
        int8_t prev = (int8_t)current - i;
        if (prev < 0) prev += limit;
        if (prev == FAULT_QTY || active_faults[prev]) {
            return prev;
        }
    }
    return current; // Si no encuentra nada, se queda donde está
}
