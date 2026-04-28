/*
 * Copyright (c) 2023 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @file   : task_menu.c
 * @date   : Set 26, 2023
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 * @version	v1.0.0
 */

/********************** inclusions *******************************************/
/* Project includes */
#include "main.h"

/* Demo includes */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes */
#include "board.h"
#include "app.h"
#include "task_menu_attribute.h"
#include "task_menu_interface.h"
#include "display.h"

/********************** macros and definitions *******************************/
#define G_TASK_MEN_CNT_INI			0ul
#define G_TASK_MEN_TICK_CNT_INI		0ul

#define DEL_MEN_XX_MIN				0ul
#define DEL_MEN_XX_MED				50ul
#define DEL_MEN_XX_MAX				500ul

#define MAX_PARAMETROS				sizeof(task_menu_parameters_t)
#define MAX_TEST					sizeof(task_menu_test_t)

/********************** internal data declaration ****************************/
task_menu_dta_t task_menu_dta_list[] = { { DEL_MEN_XX_MIN, ST_SYS_00,
		EV_SYS_BTN_ESC, false, PARAM_HUM_SUELO, 0, TEST_HUM_SUELO }, {
DEL_MEN_XX_MIN, ST_SYS_00, EV_SYS_BTN_ESC, false, PARAM_HUM_SUELO, 0,
		TEST_HUM_SUELO }, { DEL_MEN_XX_MIN, ST_SYS_00, EV_SYS_BTN_ESC, false,
		PARAM_HUM_SUELO, 0, TEST_HUM_SUELO }, { DEL_MEN_XX_MIN, ST_SYS_00,
		EV_SYS_BTN_ESC, false, PARAM_HUM_SUELO, 0, TEST_HUM_SUELO } };

motor_info_t motor_info_list[] = { { false, 0, false }, { false, 0, false } };

const int MAX_VAL[] = { MAX_POWER, MAX_SPEED, MAX_SPIN };
const int MIN_VAL[] = { MIN_POWER, MIN_SPEED, MIN_SPIN };

const char *var_names[] = { "Power", "Speed", "Spin" };
const char *val_names[][10] = { { "OFF", "ON" }, { "0", "1", "2", "3", "4", "5",
		"6", "7", "8", "9" }, { "L", "R" } };

#define MENU_DTA_QTY	(sizeof(task_menu_dta_list)/sizeof(task_menu_dta_t))

/********************** internal functions declaration ***********************/
void task_menu_statechart_normal(void);
void task_menu_statechart_setup(void);
void task_menu_statechart_failure(void);
void task_menu_statechart_test(void);
int get_value(task_menu_parameters_t parameter);
void set_value(task_menu_parameters_t parameter, int value);
void test_function(task_menu_test_t current_test);
void LCD_show(const char *first_row, const char *second_row);

/********************** internal data definition *****************************/
const char *p_task_menu = "Task Menu (Interactive Menu)";
const char *p_task_menu_ = "Non-Blocking & Update By Time Code";

/********************** external data declaration ****************************/
uint32_t g_task_menu_cnt;
task_menu_sys_t active_system;
volatile uint32_t g_task_menu_tick_cnt;

/********************** external functions definition ************************/

void LCD_show(const char *first_row, const char *second_row) {
	displayCharPositionWrite(0, 0);
	displayStringWrite("                    ");
	displayCharPositionWrite(0, 1);
	displayStringWrite("                    ");
	displayCharPositionWrite(0, 0);
	displayStringWrite(first_row);
	displayCharPositionWrite(0, 1);
	displayStringWrite(second_row);
}

void task_menu_init(void *parameters) {
	task_menu_dta_t *p_task_menu_dta;
	task_menu_st_t state;
	task_menu_ev_t event;
	bool b_event;

	char first_row[20];
	char second_row[20];
	/* Print out: Task Initialized */
	LOGGER_INFO(" ");
	LOGGER_INFO("  %s is running - %s", GET_NAME(task_menu_init), p_task_menu);
	LOGGER_INFO("  %s is a %s", GET_NAME(task_menu), p_task_menu_);

	/* Init & Print out: Task execution counter */
	g_task_menu_cnt = G_TASK_MEN_CNT_INI;
	LOGGER_INFO("   %s = %lu", GET_NAME(g_task_menu_cnt), g_task_menu_cnt);

	active_system = SYS_NORMAL;

	init_queue_event_task_menu();

	for (index = 0; MENU_DTA_QTY > index; index++) {

		/* Update Task Actuator Configuration & Data Pointer */
		p_task_menu_dta = &task_menu_dta_list[index];

		/* Init & Print out: Task execution FSM */
		state = ST_SYS_00;
		p_task_menu_dta->state = state;

		event = EV_SYS_BTN_ESC;
		p_task_menu_dta->event = event;

		b_event = false;
		p_task_menu_dta->flag = b_event;

		LOGGER_INFO(" ");
		LOGGER_INFO("   %s = %lu   %s = %lu   %s = %s", GET_NAME(state),
				(uint32_t )state, GET_NAME(event), (uint32_t )event,
				GET_NAME(b_event), (b_event ? "true" : "false"));
	}

	/* Init & Print out: LCD Display */
	displayInit(DISPLAY_CONNECTION_GPIO_4BITS);
	snprintf(first_row, sizeof(first_row), "Motor 0: %s, %s, %s",
			val_names[0][motor_info_list[0].power],
			val_names[1][motor_info_list[0].speed],
			val_names[2][motor_info_list[0].spin]);
	snprintf(second_row, sizeof(second_row), "Motor 1: %s, %s, %s",
			val_names[0][motor_info_list[1].power],
			val_names[1][motor_info_list[1].speed],
			val_names[2][motor_info_list[1].spin]);
	LCD_show(first_row, second_row);
}

void task_menu_update(void *parameters) {
	bool b_time_update_required = false;

	/* Protect shared resource */
	__asm("CPSID i");
	/* disable interrupts */
	if (G_TASK_MEN_TICK_CNT_INI < g_task_menu_tick_cnt) {
		/* Update Tick Counter */
		g_task_menu_tick_cnt--;
		b_time_update_required = true;
	}
	__asm("CPSIE i");
	/* enable interrupts */

	while (b_time_update_required) {
		/* Update Task Counter */
		g_task_menu_cnt++;

		/* Run Task Menu Statechart */
		switch (active_system) {
		case SYS_NORMAL:
			task_menu_statechart_normal();
			break;
		case SYS_SETUP:
			task_menu_statechart_setup();
			break;
		case SYS_FAILURE:
			task_menu_statechart_failure();
			break;
		case SYS_TEST:
			task_menu_statechart_test();
			break;
		default:
			break;
		}

		/* Protect shared resource */
		__asm("CPSID i");
		/* disable interrupts */
		if (G_TASK_MEN_TICK_CNT_INI < g_task_menu_tick_cnt) {
			/* Update Tick Counter */
			g_task_menu_tick_cnt--;
			b_time_update_required = true;
		} else {
			b_time_update_required = false;
		}
		__asm("CPSIE i");
		/* enable interrupts */
	}
}

void task_menu_statechart_normal(void) {
	task_menu_dta_t *p_task_menu_dta;
	char first_row[20];
	char second_row[20];

	/* Update Task Menu Data Pointer */
	p_task_menu_dta = &task_menu_dta_list[active_system];

	if (true == any_event_task_menu()) {
		p_task_menu_dta->flag = true;
		p_task_menu_dta->event = get_event_task_menu();
	}

	//TODO: rehacer cada caso. Reahcer lo del LCD.
	switch (p_task_menu_dta->state) {
	case ST_SYS_00: //Humedad Suelo
		if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_RIGHT == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_01;
			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_LEFT == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_04;
			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_ENTER == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_00;
			active_system = SYS_SETUP;
			p_task_menu_dta->current_parameter = PARAM_HUM_SUELO;

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_ESC_HOLD == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_00;
			active_system = SYS_TEST;
			p_task_menu_dta->current_parameter = TEST_NIVEL_AGUA;

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_NEXT_TIMER == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_01;

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		}

		break;

	case ST_SYS_01: // Humedad Ambiente
		if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_RIGHT == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_02;
			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_LEFT == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_00;
			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_ENTER == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_01;
			active_system = SYS_SETUP;
			p_task_menu_dta->current_parameter = PARAM_HUM_AMB;

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_ESC_HOLD == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_01;
			active_system = SYS_TEST;
			p_task_menu_dta->current_parameter = TEST_BOMBA;

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_NEXT_TIMER == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_02;

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		}
		break;

	case ST_SYS_02: // Temperatura ambiente
		if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_RIGHT == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_03;
			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_LEFT == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_01;
			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_ENTER == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_02;
			active_system = SYS_SETUP;
			p_task_menu_dta->current_parameter = PARAM_TEMP_AMB;

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_ESC_HOLD == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_02;
			active_system = SYS_TEST;
			p_task_menu_dta->current_parameter = TEST_LUZ;

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_NEXT_TIMER == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_03;

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		}
		break;
	case ST_SYS_03: // Luz
		if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_RIGHT == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_04;
			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_LEFT == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_02;
			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_ENTER == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_03;
			active_system = SYS_SETUP;
			p_task_menu_dta->current_parameter = PARAM_LUZ;

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_ESC_HOLD == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_03;
			active_system = SYS_TEST;
			p_task_menu_dta->current_parameter = TEST_LUZ;

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_NEXT_TIMER == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_04;

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		}
		break;
	case ST_SYS_04: // Nivel de Agua
		if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_RIGHT == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_00;
			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_LEFT == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_03;
			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_ENTER == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_04;
			active_system = SYS_SETUP;
			p_task_menu_dta->current_parameter = PARAM_AGUA;

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_ESC_HOLD == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_04;
			active_system = SYS_TEST;
			p_task_menu_dta->current_parameter = TEST_LED;

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_NEXT_TIMER == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_00;

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Motor:", second_row);
		}
		break;

	default:

		break;
	}
}

//TODO: verificar el funcionamiento y ver lo de la pantalla
void task_menu_statechart_setup(void) {
	task_menu_dta_t *p_task_menu_dta;
	char first_row[20];
	char second_row[20];

	/* Update Task Menu Data Pointer */
	p_task_menu_dta = &task_menu_dta_list[active_system];

	if (true == any_event_task_menu()) {
		p_task_menu_dta->flag = true;
		p_task_menu_dta->event = get_event_task_menu();
	}

	switch (p_task_menu_dta->state) {
	case ST_SYS_00: //Menu Setup General: se elige que parametro modificar

		if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_RIGHT == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_00;

			if (p_task_menu_dta->current_parameter < MAX_PARAMETROS) {
				p_task_menu_dta->current_parameter++;
			} else {
				p_task_menu_dta->current_parameter = 0;
			}

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Parameter:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_LEFT == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_00;

			if (p_task_menu_dta->current_parameter > 0) {
				p_task_menu_dta->current_parameter--;
			} else {
				p_task_menu_dta->current_parameter = MAX_PARAMETROS;
			}

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Parameter:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_ENTER == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_01;

			p_task_menu_dta->current_value = get_value(
					p_task_menu_dta->current_parameter);

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_value);
			LCD_show("Select Value:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_ESC == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_00;
			active_system = SYS_NORMAL;

			//TODO: ver (deberia mostrar la pantalla en normal)
			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Parameter:", second_row);
		}

		break;
	case ST_SYS_01: //Menu Setup de cada parametro: se elige el valor del parametro.

		if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_RIGHT == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_01;

			if (p_task_menu_dta->current_value
					< MAX_VAL[p_task_menu_dta->current_parameter]) {
				p_task_menu_dta->current_value++;
			} else {
				p_task_menu_dta->current_value =
						MIN_VAL[p_task_menu_dta->current_parameter];
			}

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_value);
			LCD_show("Select Value:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_LEFT == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_01;

			if (p_task_menu_dta->current_value
					>= MIN_VAL[p_task_menu_dta->current_parameter]) {
				p_task_menu_dta->current_value--;
			} else {
				p_task_menu_dta->current_value =
						MAX_VAL[p_task_menu_dta->current_parameter];
			}

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_value);
			LCD_show("Select Value:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_ENTER == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_00;

			save_value(p_task_menu_dta->current_parameter,
					p_task_menu_dta->current_value);

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Parameter:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_ESC == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_00;

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Value:", second_row);
		}

		break;

	default:

		break;
	}
}

void task_menu_statechart_failure(void) {
}

void task_menu_statechart_test(void) {
	task_menu_dta_t *p_task_menu_dta;
	char first_row[20];
	char second_row[20];

	/* Update Task Menu Data Pointer */
	p_task_menu_dta = &task_menu_dta_list[active_system];

	if (true == any_event_task_menu()) {
		p_task_menu_dta->flag = true;
		p_task_menu_dta->event = get_event_task_menu();
	}

	switch (p_task_menu_dta->state) {
	case ST_SYS_00:

		if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_RIGHT == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_00;

			if (p_task_menu_dta->current_test < MAX_TEST) {
				p_task_menu_dta->current_test++;
			} else {
				p_task_menu_dta->current_test = 0;
			}

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_test);
			LCD_show("Select Parameter:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_LEFT == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_00;

			if (p_task_menu_dta->current_test > 0) {
				p_task_menu_dta->current_test--;
			} else {
				p_task_menu_dta->current_test = MAX_TEST;
			}

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Parameter:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_ENTER == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_00;

			test_function(p_task_menu_dta->current_test);

			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_value);
			LCD_show("Select Value:", second_row);
		} else if ((true == p_task_menu_dta->flag)
				&& (EV_SYS_BTN_ESC == p_task_menu_dta->event)) {
			p_task_menu_dta->flag = false;
			p_task_menu_dta->state = ST_SYS_00;
			active_system = SYS_NORMAL;

			//TODO: ver (deberia mostrar la pantalla en normal)
			snprintf(second_row, sizeof(second_row), "> %lu",
					p_task_menu_dta->current_parameter);
			LCD_show("Select Parameter:", second_row);
		}

		break;

	}

}

int get_value(task_menu_parameters_t parameter) {
	//TODO
	switch (parameter) {
	case PARAM_HUM_SUELO:

		break;
	case PARAM_HUM_AMB:

		break;
	case PARAM_TEMP_AMB:

		break;
	case PARAM_LUZ:

		break;
	case PARAM_AGUA:

		break;
	default:
		break;
	}
}

void set_value(task_menu_parameters_t parameter, int value) {
	//TODO
	switch (parameter) {
	case PARAM_HUM_SUELO:

		break;
	case PARAM_HUM_AMB:

		break;
	case PARAM_TEMP_AMB:

		break;
	case PARAM_LUZ:

		break;
	case PARAM_AGUA:

		break;
	default:
		break;
	}
}

void test_function(task_menu_test_t current_test) {
	//TODO
	switch (current_test) {
	case TEST_NIVEL_AGUA:

		break;
	case TEST_BOMBA:

		break;
	case TEST_LUZ:

		break;
	case TEST_LED:

		break;
	case TEST_HUM_SUELO:

		break;
	case TEST_TEMP:

		break;
	case TEST_BUZZER:

		break;
	default:
		break;
	}
}

/********************** end of file ******************************************/
