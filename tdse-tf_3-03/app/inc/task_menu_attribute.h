#ifndef TASK_INC_TASK_MENU_ATTRIBUTE_H_
#define TASK_INC_TASK_MENU_ATTRIBUTE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* Events to excite Task Menu */
typedef enum task_menu_ev {
	//EV_SYS_NONE,
	EV_SYS_BTN_RIGHT,
	EV_SYS_BTN_LEFT,
	EV_SYS_BTN_ENTER,
	EV_SYS_BTN_ESC,
	EV_SYS_BTN_ESC_HOLD
} task_menu_ev_t;

/* State of Task Menu */
typedef enum task_menu_st {
	ST_SYS_00, ST_SYS_01, ST_SYS_02, ST_SYS_03, ST_SYS_04,
} task_menu_st_t;

typedef enum task_menu_sys {
	SYS_NORMAL, SYS_SETUP, SYS_FAILURE, SYS_TEST,
} task_menu_sys_t;

/* Parámetros configurables de la Smartceta */
typedef enum task_menu_parameters {
	PARAM_HUM_SUELO = 0,
	PARAM_HUM_AMB,
	PARAM_TEMP_AMB,
	PARAM_LUZ,
	PARAM_AGUA,
	PARAM_QTY //Para saber la cantidad de parámetros
} task_menu_parameters_t;

/* Componentes para el Modo Test */
typedef enum task_menu_test {
	TEST_NIVEL_AGUA = 0,
	TEST_BOMBA,
	TEST_LUZ,
	TEST_LED,
	TEST_HUM_SUELO,
	TEST_TEMP,
	TEST_BUZZER,
	TEST_QTY, // Para saber la cantidad de tests
	TEST_NONE
} task_menu_test_t;

typedef struct {
	uint32_t tick;
	task_menu_st_t state;
	task_menu_ev_t event;
	bool flag;
	task_menu_parameters_t current_parameter;
	uint32_t current_value;
	task_menu_test_t current_test;
} task_menu_dta_t;

extern task_menu_dta_t task_menu_dta_list[];

#ifdef __cplusplus
}
#endif

#endif /* TASK_INC_TASK_MENU_ATTRIBUTE_H_ */
