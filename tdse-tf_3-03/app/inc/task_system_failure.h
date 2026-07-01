#ifndef TASK_INC_TASK_SYSTEM_FAILURE_H_
#define TASK_INC_TASK_SYSTEM_FAILURE_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif



typedef enum {
    FAULT_PUMP_OVERCURRENT,			// Exceso de corriente en bomba
    FAULT_PUMP_OPEN,				// Falta de corriente en bomba
    FAULT_LED_STRIP_OVERCURRENT,	// Exceso de corriente en tira LED
    FAULT_LED_STRIP_OPEN,			// Falta de corriente en tira LED
	FAULT_HIGH_TEMPERATURE,			// Temperatura alta
	FAULT_LOW_TEMPERATURE, 			// Temperatura baja
    FAULT_DHT22_NO_RESPONSE,		// Falta de respuesta del DHT22
    FAULT_WATER_LEVEL_LOW,			// Nivel de agua bajo
    FAULT_WATER_LEVEL_ERROR,		// Error en la lectura del sensor de nivel de agua (por ejemplo, mide 0 constantemente)
    FAULT_LIGHT_LEVEL_ERROR,		// Error en la lectura del sensor de nivel de luz (por ejemplo, mide 0 constantemente)
    FAULT_HUMIDITY_LEVEL_ERROR,		// Error en la lectura del sensor de nivel de humedad de suelo (por ejemplo, mide 0 constantemente)
// Fin de fallas conocidas, el resto son variables auxiliares
// Agregar fallas por encima de este comentario
    FAULT_QTY,						// Cantidad de fallas conocidas
	FAULT_RESTORE,					// Variable auxiliar para restaurar el sistema
	FAULT_NONE
} system_failure_type;

/********************** external functions declaration ***********************/
extern void task_system_failure_init(void *parameters);
extern void task_system_failure_update(void *parameters);
extern void task_system_failure_report(system_failure_type failure);
extern bool task_system_failure_is_locked(void);
extern bool task_system_failure_can_restore(void);
extern const char* task_system_failure_get_name(system_failure_type failure);
extern void task_system_failure_clear_all(void);
extern system_failure_type task_system_failure_get_prev(system_failure_type failure);
extern system_failure_type task_system_failure_get_next(system_failure_type failure);
extern system_failure_type task_system_failure_get_valid_fault(system_failure_type failure);

#ifdef __cplusplus
}
#endif

#endif /* TASK_INC_TASK_SYSTEM_FAILURE_H_ */

/********************** end of file ******************************************/
