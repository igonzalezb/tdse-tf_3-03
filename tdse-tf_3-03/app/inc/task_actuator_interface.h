#ifndef TASK_INC_TASK_ACTUATOR_INTERFACE_H_
#define TASK_INC_TASK_ACTUATOR_INTERFACE_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/
#include <stdbool.h>
#include "task_actuator_attribute.h"

/********************** external functions declaration ***********************/
extern void init_queue_event_task_actuator(void);
extern void put_event_task_actuator(task_actuator_id_t id, uint32_t event);
extern uint32_t get_event_task_actuator(task_actuator_id_t id);
extern bool any_event_task_actuator(task_actuator_id_t id);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_INC_TASK_ACTUATOR_INTERFACE_H_ */
/********************** end of file ******************************************/
