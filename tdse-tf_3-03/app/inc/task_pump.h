#ifndef TASK_INC_TASK_PUMP_H_
#define TASK_INC_TASK_PUMP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "task_actuator_attribute.h"
/********************** external functions declaration ***********************/
extern void task_pump_init(void *parameters);
extern void task_pump_update(void *parameters);
extern task_pump_st_t get_pump_state(void);

#ifdef __cplusplus
}
#endif

#endif /* TASK_INC_TASK_PUMP_H_ */

/********************** end of file ******************************************/
