/*
 * Copyright (c) 2023 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * @file   : task_button.h
 */

#ifndef TASK_INC_TASK_BUTTON_H_
#define TASK_INC_TASK_BUTTON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/********************** external data declaration ****************************/
extern uint32_t g_task_button_cnt;
extern volatile uint32_t g_task_button_tick_cnt;

/********************** external functions declaration ***********************/
extern void task_button_init(void *parameters);
extern void task_button_update(void *parameters);

#ifdef __cplusplus
}
#endif

#endif /* TASK_INC_TASK_BUTTON_H_ */

/********************** end of file ******************************************/
