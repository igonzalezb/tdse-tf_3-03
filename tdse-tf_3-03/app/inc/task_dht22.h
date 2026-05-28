#ifndef TASK_INC_TASK_DHT22_H_
#define TASK_INC_TASK_DHT22_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void task_dht22_init(void *parameters);
void task_dht22_update(void *parameters);
void task_dht22_exti_callback(uint16_t GPIO_Pin);

#ifdef __cplusplus
}
#endif

#endif /* TASK_INC_TASK_DHT22_H_ */
