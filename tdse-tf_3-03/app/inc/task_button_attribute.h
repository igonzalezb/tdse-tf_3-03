/*
 * Copyright (c) 2023 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * @file   : task_button_attribute.h
 */

#ifndef TASK_INC_TASK_BUTTON_ATTRIBUTE_H_
#define TASK_INC_TASK_BUTTON_ATTRIBUTE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "stm32f1xx_hal.h"

/*
 * Valor usado cuando un botón no debe generar evento de menú
 * al subir, bajar o mantener presionado.
 */
#define TASK_BUTTON_NO_SIGNAL    ((int32_t)-1)

/* Eventos internos del antirrebote del botón */
typedef enum task_button_ev {
	EV_BTN_XX_UP,
	EV_BTN_XX_DOWN
} task_button_ev_t;

/* Estados internos del antirrebote del botón */
typedef enum task_button_st {
	ST_BTN_XX_UP,
	ST_BTN_XX_FALLING,
	ST_BTN_XX_DOWN,
	ST_BTN_XX_RISING
} task_button_st_t;

/* Identificador de cada botón físico */
typedef enum task_button_id {
	ID_BTN_ENT,
	ID_BTN_NEX,
	ID_BTN_PREV,
	ID_BTN_ESC
} task_button_id_t;

typedef struct {
	task_button_id_t identifier;
	GPIO_TypeDef *gpio_port;
	uint16_t pin;
	GPIO_PinState pressed;
	uint32_t tick_max;
	uint32_t tick_hold_max;

	/*
	 * Señales enviadas al task_menu.
	 * Se guardan como int32_t para poder usar TASK_BUTTON_NO_SIGNAL.
	 * Cuando hay señal válida, se castea a task_menu_ev_t antes de enviarla.
	 */
	int32_t signal_up;
	int32_t signal_down;
	int32_t signal_hold;
} task_button_cfg_t;

typedef struct {
	uint32_t tick;
	uint32_t tick_hold;
	task_button_st_t state;
	task_button_ev_t event;
} task_button_dta_t;

/********************** external data declaration ****************************/
extern task_button_dta_t task_button_dta_list[];

#ifdef __cplusplus
}
#endif

#endif /* TASK_INC_TASK_BUTTON_ATTRIBUTE_H_ */

/********************** end of file ******************************************/
