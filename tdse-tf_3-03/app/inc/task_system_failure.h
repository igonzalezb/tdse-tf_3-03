#ifndef TASK_INC_TASK_SYSTEM_FAILURE_H_
#define TASK_INC_TASK_SYSTEM_FAILURE_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Para añadir nuevas fallas:
 * 1) Incorporarlas en el enum system_failure_type en task_system_failure.h
 * 2) Agregar una etiqueta en fault_names[FAULT_QTY] EN ORDEN en task_system_failure.c
 * 3) Agregarla en task_system_failure_update() en task_system_failure.c
 * 4) Agregar una forma de verificar si se solucionó en task_system_failure_can_restore en task_system_failure.c
 */

/*** Valores maximos ***/
#define MAX_TEMPERATURE 35
#define MIN_TEMPERATURE 0
#define MAX_PUMP_CURRENT 300
#define MIN_PUMP_CURRENT 10
#define MAX_LED_STRIP_CURRENT 200
#define MIN_LED_STRIP_CURRENT 30

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
/**** Espacio para añadir nuevas fallas:****/
	FAULT_PUMP_DRIVER,				// Driver de bomba en corto (MOSFET)
	FAULT_LED_STRIP_DRIVER,			// Driver de tira LED en corto (MOSFET)

/**** Fin Espacio para añadir nuevas fallas****/
// Variables auxiliares
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
