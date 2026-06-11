#ifndef TASK_INC_TASK_ACTUATOR_INTERFACE_H_
#define TASK_INC_TASK_ACTUATOR_INTERFACE_H_

#include <stdbool.h>
#include "task_actuator_attribute.h"

extern void init_queue_event_task_actuator(void);
extern void put_event_task_actuator(task_actuator_id_t id, uint32_t event);
uint32_t get_event_task_actuator(task_actuator_id_t id);
bool any_event_task_actuator(task_actuator_id_t id);

#endif /* TASK_INC_TASK_ACTUATOR_INTERFACE_H_ */
