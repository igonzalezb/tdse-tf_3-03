#ifndef TASK_INC_TASK_SYSTEM_FAILURE_H_
#define TASK_INC_TASK_SYSTEM_FAILURE_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif



typedef enum {
    FAULT_PUMP_OVERCURRENT,
    FAULT_PUMP_OPEN,
    FAULT_LED_STRIP_OVERCURRENT,
    FAULT_LED_STRIP_OPEN,
	FAULT_HIGH_TEMPERATURE,
	FAULT_LOW_TEMPERATURE,
    FAULT_DHT22_NO_RESPONSE,
    FAULT_WATER_LEVEL_LOW,
    FAULT_WATER_LEVEL_ERROR,
    FAULT_LIGHT_LEVEL_ERROR,
    FAULT_HUMIDITY_LEVEL_ERROR,
    FAULT_QTY
} system_failure_type;

/********************** external functions declaration ***********************/
extern void task_system_failure_init(void *parameters);
extern void task_system_failure_update(void *parameters);
extern void task_system_failure_report(system_failure_type failure);
extern bool task_system_failure_is_locked(void);
extern bool task_system_failure_can_restore(void);
extern const char* task_system_failure_get_name(system_failure_type failure);
extern void task_system_failure_clear_all(void);
extern system_failure_type task_system_failure_get_prev(system_failure_type current);
extern system_failure_type task_system_failure_get_next(system_failure_type current);
extern system_failure_type task_system_failure_get_valid_fault(system_failure_type failure);

#ifdef __cplusplus
}
#endif

#endif /* TASK_INC_TASK_SYSTEM_FAILURE_H_ */

/********************** end of file ******************************************/
