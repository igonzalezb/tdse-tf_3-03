#include "main.h"
#include "logger.h"
#include "dwt.h"
#include "board.h"
#include "app.h"
#include "task_menu_attribute.h"
#include "task_menu_interface.h"
#include "task_actuator_interface.h"
#include "display.h"

#include "stm32f1xx_hal.h"

// Dirección de la Página 127 (última página de 1KB en la STM32F103RB)
#define FLASH_CONFIG_ADDRESS 0x0801FC00

#define G_TASK_MEN_CNT_INI          0ul
#define G_TASK_MEN_TICK_CNT_INI     0ul
#define DEL_MEN_XX_MIN              0ul
#define AUTO_SCROLL_DELAY 			5000

/* Límites de configuración (Ej: 0% a 100%) */
//todo: integrarlo con shared data
const uint32_t MAX_VAL[PARAM_QTY] = { 100, 100, 50, 100, 100 };
const uint32_t MIN_VAL[PARAM_QTY] = { 0, 0, 0, 0, 0 };

static uint32_t last_interaction_tick = AUTO_SCROLL_DELAY;

/* Nombres para mostrar en el LCD */
const char *param_names[PARAM_QTY];
const char *test_names[TEST_QTY];

//todo: ponerlo en init
uint32_t config_values[PARAM_QTY] = { 40, 50, 25, 60, 20 }; // Valores por defecto

task_menu_dta_t task_menu_dta_list[] = {
		{ DEL_MEN_XX_MIN, ST_SYS_00, EV_SYS_BTN_ESC, false, PARAM_HUM_SUELO, 0, TEST_WATER_LEVEL },
		{ DEL_MEN_XX_MIN, ST_SYS_00, EV_SYS_BTN_ESC, false, PARAM_HUM_SUELO, 0,	TEST_WATER_LEVEL },
		{ DEL_MEN_XX_MIN, ST_SYS_00, EV_SYS_BTN_ESC, false,	PARAM_HUM_SUELO, 0, TEST_WATER_LEVEL },
		{ DEL_MEN_XX_MIN, ST_SYS_00, EV_SYS_BTN_ESC, false, PARAM_HUM_SUELO, 0, TEST_WATER_LEVEL } };

#define MENU_DTA_QTY    (sizeof(task_menu_dta_list)/sizeof(task_menu_dta_t))

void task_menu_statechart_normal(void);
void task_menu_statechart_setup(void);
void task_menu_statechart_failure(void);
void task_menu_statechart_test(void);
uint32_t get_value(task_menu_parameters_t parameter);
void set_value(task_menu_parameters_t parameter, uint32_t value);
void test_function(task_menu_test_t current_test);
void LCD_show(const char *first_row, const char *second_row);

uint32_t g_task_menu_cnt;

volatile uint32_t g_task_menu_tick_cnt;

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
	uint32_t index;
	task_menu_dta_t *p_task_menu_dta;

	g_task_menu_cnt = G_TASK_MEN_CNT_INI;
	shared_data.active_system = SYS_NORMAL;
	shared_data.active_test = TEST_NONE;
	init_queue_event_task_menu();

	displayInit(DISPLAY_CONNECTION_GPIO_4BITS);
	LCD_show("    SMARTCETA   ", "   Iniciando...  ");

	for (index = 0; MENU_DTA_QTY > index; index++) {
		p_task_menu_dta = &task_menu_dta_list[index];
		p_task_menu_dta->state = ST_SYS_00;
		p_task_menu_dta->event = EV_SYS_BTN_ESC;
		p_task_menu_dta->flag = false;
		p_task_menu_dta->current_parameter = PARAM_HUM_SUELO;
		p_task_menu_dta->current_test = TEST_WATER_LEVEL;
	}

	param_names[PARAM_HUM_SUELO] = "Hum. Suelo";
	param_names[PARAM_HUM_AMB] = "Hum. Amb.";
	param_names[PARAM_TEMP_AMB] = "Temp. Amb.";
	param_names[PARAM_AGUA] = "Nivel Agua";
	param_names[PARAM_LUZ] = "Luz";

	test_names[TEST_WATER_LEVEL] = "Test Agua";
	test_names[TEST_LIGHT] = "Test Luz";
	test_names[TEST_HUMIDITY] = "Test H.Suelo";
	test_names[TEST_DHT22] = "Test DHT22";
	test_names[TEST_STATE_LED] = "Test LED";
	test_names[TEST_BUZZER] = "Test Buzzer";
	test_names[TEST_PUMP] = "Test Bomba";
	test_names[TEST_LED_STRIP] = "Test T. LED";





	/* Configuracion en memoria*/
	uint32_t humedad_suelo_guardada = get_value(PARAM_HUM_SUELO);
	if (humedad_suelo_guardada == 0xFFFFFFFF) {
		LOGGER_INFO("PRIMERA VER CONFIGURANDO");
		for (int i = 0; i < PARAM_QTY; i++) {
			set_value(i, config_values[i]);
		}
	} else {
		LOGGER_INFO("OBTENIENDO VALORES DE MEMORIA");
		for (int i = 0; i < PARAM_QTY; i++) {
			config_values[i] = get_value(i);
		}
	}

}

void task_menu_update(void *parameters) {
	bool b_time_update_required = false;

	__asm("CPSID i");
	if (G_TASK_MEN_TICK_CNT_INI < g_task_menu_tick_cnt) {
		g_task_menu_tick_cnt--;
		b_time_update_required = true;
	}
	__asm("CPSIE i");

	while (b_time_update_required) {
		g_task_menu_cnt++;

		switch (shared_data.active_system) {
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
		/******************************************************************/
		/* Codigo para probar el led y el buzzer mediante eventos, borrar */
		static task_menu_sys_t menu_last_system = (task_menu_sys_t)-1;

		    if (shared_data.active_system != menu_last_system) {
		        menu_last_system = shared_data.active_system;

		        // Mandamos el evento por la cola
		        if (shared_data.active_system == SYS_FAILURE) {
		            put_event_task_actuator(ID_ACT_BUZZER, EV_BUZZER_INTERMITTENT);
		        } else {
		            put_event_task_actuator(ID_ACT_BUZZER, EV_BUZZER_PULSE);
		        }

			}



		/******************************************************************/

		__asm("CPSID i");
		if (G_TASK_MEN_TICK_CNT_INI < g_task_menu_tick_cnt) {
			g_task_menu_tick_cnt--;
			b_time_update_required = true;
		} else {
			b_time_update_required = false;
		}
		__asm("CPSIE i");
	}
}

void task_menu_statechart_normal(void) {
	task_menu_dta_t *p_task_menu_dta = &task_menu_dta_list[shared_data.active_system];
	char second_row[20];

	if (true == any_event_task_menu()) {
		p_task_menu_dta->flag = true;
		p_task_menu_dta->event = get_event_task_menu();
	}

	if (p_task_menu_dta->flag) {
		p_task_menu_dta->flag = false;

		// Navegación entre sensores (derecha/izquierda)
		if (p_task_menu_dta->event == EV_SYS_BTN_RIGHT) {
			LOGGER_INFO("BTN_RIGHT PRESSED");
			p_task_menu_dta->state =
					(p_task_menu_dta->state == ST_SYS_04) ?
							ST_SYS_00 : p_task_menu_dta->state + 1;
		} else if (p_task_menu_dta->event == EV_SYS_BTN_LEFT) {
			LOGGER_INFO("BTN_LEFT PRESSED");
			p_task_menu_dta->state =
					(p_task_menu_dta->state == ST_SYS_00) ?
							ST_SYS_04 : p_task_menu_dta->state - 1;
		}
		// Ingreso a Setup
		else if (p_task_menu_dta->event == EV_SYS_BTN_ENTER) {
			LOGGER_INFO("BTN_ENTER PRESSED");
			shared_data.active_system = SYS_SETUP;
			p_task_menu_dta->state = ST_SYS_00; // Reiniciar estado de setup
			p_task_menu_dta->current_parameter = PARAM_HUM_SUELO;
			LCD_show("Configurar:",
					param_names[p_task_menu_dta->current_parameter]);
			return;
		}
		// Ingreso a Test
		else if (p_task_menu_dta->event == EV_SYS_BTN_ESC_HOLD) {
			LOGGER_INFO("BTN_ESC HOLD");
			shared_data.active_system = SYS_TEST;
			p_task_menu_dta->state = ST_SYS_00;
			p_task_menu_dta->current_test = TEST_WATER_LEVEL;
			LCD_show("Modo Test:", test_names[p_task_menu_dta->current_test]);
			return;
		}
		last_interaction_tick = HAL_GetTick();
		// Actualizar pantalla según el estado actual
		// TODO: Leer los valores reales desde la estructura shared_data
		snprintf(second_row, sizeof(second_row), "--");
		LCD_show(param_names[p_task_menu_dta->state], second_row);
	} else if ((HAL_GetTick() - last_interaction_tick) >= AUTO_SCROLL_DELAY) {
		p_task_menu_dta->state =
				(p_task_menu_dta->state == ST_SYS_04) ?
						ST_SYS_00 : p_task_menu_dta->state + 1;
		snprintf(second_row, sizeof(second_row), "--");
		LCD_show(param_names[p_task_menu_dta->state], second_row);
		last_interaction_tick = HAL_GetTick();
	}
}

void task_menu_statechart_setup(void) {
	task_menu_dta_t *p_task_menu_dta = &task_menu_dta_list[shared_data.active_system];
	char second_row[20];

	if (true == any_event_task_menu()) {
		p_task_menu_dta->flag = true;
		p_task_menu_dta->event = get_event_task_menu();
	}

	switch (p_task_menu_dta->state) {
	case ST_SYS_00: // Selección de parámetro a configurar
		if (p_task_menu_dta->flag) {
			p_task_menu_dta->flag = false;

			if (p_task_menu_dta->event == EV_SYS_BTN_RIGHT) {
				LOGGER_INFO("BTN_RIGHT PRESSED");
				p_task_menu_dta->current_parameter =
						(p_task_menu_dta->current_parameter + 1) % PARAM_QTY;
				LCD_show("Configurar:",
						param_names[p_task_menu_dta->current_parameter]);
			} else if (p_task_menu_dta->event == EV_SYS_BTN_LEFT) {
				LOGGER_INFO("BTN_LEFT PRESSED");
				p_task_menu_dta->current_parameter =
						(p_task_menu_dta->current_parameter == 0) ?
								(PARAM_QTY - 1) :
								(p_task_menu_dta->current_parameter - 1);
				LCD_show("Configurar:",
						param_names[p_task_menu_dta->current_parameter]);
			} else if (p_task_menu_dta->event == EV_SYS_BTN_ENTER) {
				LOGGER_INFO("BTN_ENTER PRESSED");
				p_task_menu_dta->state = ST_SYS_01;
				snprintf(second_row, sizeof(second_row), "> %lu",
						config_values[p_task_menu_dta->current_parameter]);
				LCD_show(param_names[p_task_menu_dta->current_parameter],
						second_row);
			} else if (p_task_menu_dta->event == EV_SYS_BTN_ESC) {
				LOGGER_INFO("BTN_ESC PRESSED");
				shared_data.active_system = SYS_NORMAL;
				p_task_menu_dta->state = ST_SYS_00; // Forzar refresco
				LCD_show("Saliendo...", "");
			}
		}
		break;

	case ST_SYS_01: // Modificación del valor del parámetro
		if (p_task_menu_dta->flag) {
			p_task_menu_dta->flag = false;

			if (p_task_menu_dta->event == EV_SYS_BTN_RIGHT) {
				LOGGER_INFO("BTN_RIGHT PRESSED");
				if (config_values[p_task_menu_dta->current_parameter]
						< MAX_VAL[p_task_menu_dta->current_parameter]) {
					config_values[p_task_menu_dta->current_parameter]++;
				}
				snprintf(second_row, sizeof(second_row), "> %lu",
						config_values[p_task_menu_dta->current_parameter]);
				LCD_show(param_names[p_task_menu_dta->current_parameter],
						second_row);
			} else if (p_task_menu_dta->event == EV_SYS_BTN_LEFT) {
				LOGGER_INFO("BTN_LEFT PRESSED");
				if (config_values[p_task_menu_dta->current_parameter]
						> MIN_VAL[p_task_menu_dta->current_parameter]) {
					config_values[p_task_menu_dta->current_parameter]--;
				}
				snprintf(second_row, sizeof(second_row), "> %lu",
						config_values[p_task_menu_dta->current_parameter]);
				LCD_show(param_names[p_task_menu_dta->current_parameter],
						second_row);
			} else if (p_task_menu_dta->event == EV_SYS_BTN_ENTER) {
				LOGGER_INFO("BTN_ENTER PRESSED");
				set_value(p_task_menu_dta->current_parameter,
						config_values[p_task_menu_dta->current_parameter]);
				p_task_menu_dta->state = ST_SYS_00;
				LCD_show("Guardado!",
						param_names[p_task_menu_dta->current_parameter]);
			} else if (p_task_menu_dta->event == EV_SYS_BTN_ESC) {
				LOGGER_INFO("BTN_ESC PRESSED");
				p_task_menu_dta->state = ST_SYS_00;
				LCD_show("Configurar:",
						param_names[p_task_menu_dta->current_parameter]);
			}
		}
		break;
	default:
		break;
	}
}

//TODO: poner active test de shared data
void task_menu_statechart_test(void) {
	task_menu_dta_t *p_task_menu_dta = &task_menu_dta_list[shared_data.active_system];

	if (true == any_event_task_menu()) {
		p_task_menu_dta->flag = true;
		p_task_menu_dta->event = get_event_task_menu();
	}

	if (p_task_menu_dta->flag) {
		p_task_menu_dta->flag = false;

		if (p_task_menu_dta->event == EV_SYS_BTN_RIGHT) {
			p_task_menu_dta->current_test = (p_task_menu_dta->current_test + 1)
					% TEST_QTY;
			LCD_show("Modo Test:", test_names[p_task_menu_dta->current_test]);
		} else if (p_task_menu_dta->event == EV_SYS_BTN_LEFT) {
			p_task_menu_dta->current_test =
					(p_task_menu_dta->current_test == 0) ?
							(TEST_QTY - 1) :
							(p_task_menu_dta->current_test - 1);
			LCD_show("Modo Test:", test_names[p_task_menu_dta->current_test]);
		} else if (p_task_menu_dta->event == EV_SYS_BTN_ENTER) {
			LCD_show("Testeando...", test_names[p_task_menu_dta->current_test]);
			shared_data.active_test = p_task_menu_dta->current_test;

		} else if (p_task_menu_dta->event == EV_SYS_BTN_ESC && shared_data.active_test != TEST_NONE) {
			shared_data.active_test = TEST_NONE;
			LCD_show("Modo Test:", test_names[p_task_menu_dta->current_test]);
		} else if (p_task_menu_dta->event == EV_SYS_BTN_ESC) {
			shared_data.active_test = TEST_NONE;
			shared_data.active_system = SYS_NORMAL;
			p_task_menu_dta->state = ST_SYS_00; // Forzar refresco
			LCD_show("Saliendo...", "");
		}
	}
}

void task_menu_statechart_failure(void) {
	// TODO: Implementar lógica de bloqueo y alertas visuales/sonoras.
}

//TODO: arreglar la lectura/escritura de memoria.
uint32_t get_value(task_menu_parameters_t parameter) {
	// Leer directamente de la memoria mapeada mediante un puntero
	uint32_t *pConfig = (uint32_t*) FLASH_CONFIG_ADDRESS;

	// Retornamos el valor usando el parámetro como índice (offset)
	return pConfig[parameter];
}

void set_value(task_menu_parameters_t parameter, uint32_t value) {
	// 1. Desbloquear la Flash para permitir modificaciones
	HAL_FLASH_Unlock();

	// 2. Configurar el borrado de la PÁGINA (Específico para F1)
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PageError = 0;

	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = FLASH_CONFIG_ADDRESS;
	EraseInitStruct.NbPages = 1; // Solo borramos nuestra página de configuración

	// Ejecutar el borrado
	HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);

	// 3. Escribir el nuevo valor
	// Calculamos el offset multiplicando el parámetro por 4 (cada uint32_t ocupa 4 bytes)
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
			FLASH_CONFIG_ADDRESS + (parameter * 4), value);

	// 4. Bloquear la Flash por seguridad para evitar escrituras accidentales
	HAL_FLASH_Lock();
}
