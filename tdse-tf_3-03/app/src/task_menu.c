#include "main.h"
#include "logger.h"
#include "dwt.h"
#include "board.h"
#include "app.h"
#include "task_menu_attribute.h"
#include "task_menu_interface.h"
#include "display.h"

#include "stm32f1xx_hal.h"

// Dirección de la Página 127 (última página de 1KB en la STM32F103RB)
#define FLASH_CONFIG_ADDRESS 0x0801FC00

#define G_TASK_MEN_CNT_INI          0ul
#define G_TASK_MEN_TICK_CNT_INI     0ul
#define DEL_MEN_XX_MIN              0ul
#define AUTO_SCROLL_DELAY 			5000

/* Límites de configuración (Ej: 0% a 100%) */
const uint32_t MAX_VAL[PARAM_QTY] = { 100, 100, 50, 100, 100 };
const uint32_t MIN_VAL[PARAM_QTY] = { 0, 0, 0, 0, 0 };

static uint32_t last_interaction_tick = AUTO_SCROLL_DELAY;

/* Nombres para mostrar en el LCD */
const char *param_names[PARAM_QTY] = { "Hum. Suelo", "Hum. Amb.", "Temp. Amb.",
		"Luz", "Nivel Agua" };
const char *test_names[TEST_QTY] = { "Test Agua", "Test Bomba", "Test Luz",
		"Test LED", "Test H.Suelo", "Test Temp", "Test Buzzer" };

uint32_t config_values[PARAM_QTY] = { 40, 50, 25, 60, 20 }; // Valores por defecto

task_menu_dta_t task_menu_dta_list[] = { { DEL_MEN_XX_MIN, ST_SYS_00,
		EV_SYS_BTN_ESC, false, PARAM_HUM_SUELO, 0, TEST_NIVEL_AGUA }, {
		DEL_MEN_XX_MIN, ST_SYS_00, EV_SYS_BTN_ESC, false, PARAM_HUM_SUELO, 0,
		TEST_NIVEL_AGUA }, { DEL_MEN_XX_MIN, ST_SYS_00, EV_SYS_BTN_ESC, false,
		PARAM_HUM_SUELO, 0, TEST_NIVEL_AGUA }, { DEL_MEN_XX_MIN, ST_SYS_00,
		EV_SYS_BTN_ESC, false, PARAM_HUM_SUELO, 0, TEST_NIVEL_AGUA } };

#define MENU_DTA_QTY    (sizeof(task_menu_dta_list)/sizeof(task_menu_dta_t))

void task_menu_statechart_normal(void);
void task_menu_statechart_setup(void);
void task_menu_statechart_failure(void);
void task_menu_statechart_test(void);
uint32_t get_value(task_menu_parameters_t parameter);
void set_value(task_menu_parameters_t parameter, uint32_t value);
void test_function(task_menu_test_t current_test);
void LCD_show(const char *first_row, const char *second_row);
static void task_menu_refresh_state_led(task_menu_sys_t current_mode);

uint32_t g_task_menu_cnt;
task_menu_sys_t active_system;
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
	active_system = SYS_NORMAL;
	init_queue_event_task_menu();

	displayInit(DISPLAY_CONNECTION_GPIO_4BITS);
	LCD_show("    SMARTCETA   ", "   Iniciando...  ");

	for (index = 0; MENU_DTA_QTY > index; index++) {
		p_task_menu_dta = &task_menu_dta_list[index];
		p_task_menu_dta->state = ST_SYS_00;
		p_task_menu_dta->event = EV_SYS_BTN_ESC;
		p_task_menu_dta->flag = false;
		p_task_menu_dta->current_parameter = PARAM_HUM_SUELO;
		p_task_menu_dta->current_test = TEST_NIVEL_AGUA;
	}
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

		task_menu_refresh_state_led(active_system);

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
	task_menu_dta_t *p_task_menu_dta = &task_menu_dta_list[active_system];
	char second_row[20];

	if (true == any_event_task_menu()) {
		p_task_menu_dta->flag = true;
		p_task_menu_dta->event = get_event_task_menu();
	}

	if (p_task_menu_dta->flag) {
		p_task_menu_dta->flag = false;

		// Navegación entre sensores (derecha/izquierda)
		if (p_task_menu_dta->event
				== EV_SYS_BTN_RIGHT /*|| p_task_menu_dta->event == EV_SYS_NEXT_TIMER*/) {
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
			active_system = SYS_SETUP;
			p_task_menu_dta->state = ST_SYS_00; // Reiniciar estado de setup
			p_task_menu_dta->current_parameter = PARAM_HUM_SUELO;
			LCD_show("Configurar:",
					param_names[p_task_menu_dta->current_parameter]);
			return;
		}
		// Ingreso a Test
		else if (p_task_menu_dta->event == EV_SYS_BTN_ESC_HOLD) {
			LOGGER_INFO("BTN_ESC HOLD");
			active_system = SYS_TEST;
			p_task_menu_dta->state = ST_SYS_00;
			p_task_menu_dta->current_test = TEST_NIVEL_AGUA;
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
	task_menu_dta_t *p_task_menu_dta = &task_menu_dta_list[active_system];
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
				active_system = SYS_NORMAL;
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

void task_menu_statechart_test(void) {
	task_menu_dta_t *p_task_menu_dta = &task_menu_dta_list[active_system];

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
			test_function(p_task_menu_dta->current_test);
			LCD_show("Test Finalizado",
					test_names[p_task_menu_dta->current_test]);
		} else if (p_task_menu_dta->event == EV_SYS_BTN_ESC) {
			active_system = SYS_NORMAL;
			p_task_menu_dta->state = ST_SYS_00; // Forzar refresco
			LCD_show("Saliendo...", "");
		}
	}
}

void task_menu_statechart_failure(void) {
	// TODO: Implementar lógica de bloqueo y alertas visuales/sonoras.
}

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

void test_function(task_menu_test_t current_test) {
	// TODO: Escribir pines de hardware para activar relés, LEDs, o leer forzadamente un ADC.
	switch (current_test) {
	case TEST_NIVEL_AGUA:
		break;
	case TEST_BOMBA:
		// Ej: HAL_GPIO_WritePin(PUMP_PORT, PUMP_PIN, GPIO_PIN_SET);
		// HAL_Delay(1000);
		// HAL_GPIO_WritePin(PUMP_PORT, PUMP_PIN, GPIO_PIN_RESET);
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


/**
 * Actualiza el estado del LED RGB según el modo del sistema.
 * Se ejecuta solo cuando hay un cambio de modo.
 */
static void task_menu_refresh_state_led(task_menu_sys_t current_mode) {
    static task_menu_sys_t last_mode = (task_menu_sys_t)-1; // Para detectar cambios de modo

    // Si el modo no cambió, sale inmediatamente
    if (current_mode == last_mode) {
        return;
    }
    last_mode = current_mode;

    // Se traduce el modo del sistema a Colores y Parpadeos
    switch (current_mode) {
        case SYS_NORMAL:
            shared_data.pwm_state_led_red   = 0;
            shared_data.pwm_state_led_green = 100;
            shared_data.pwm_state_led_blue  = 0;
            shared_data.blinking_rate       = STATE_LED_NO_BLINK;
            break;

        case SYS_SETUP:
            shared_data.pwm_state_led_red   = 0;
            shared_data.pwm_state_led_green = 0;
            shared_data.pwm_state_led_blue  = 100;
            shared_data.blinking_rate       = STATE_LED_SLOW_BLINK;
            break;

        case SYS_FAILURE:
            shared_data.pwm_state_led_red   = 100;
            shared_data.pwm_state_led_green = 0;
            shared_data.pwm_state_led_blue  = 0;
            shared_data.blinking_rate       = STATE_LED_FAST_BLINK;
            break;

        case SYS_TEST:
            shared_data.pwm_state_led_red   = 50;
            shared_data.pwm_state_led_green = 0;
            shared_data.pwm_state_led_blue  = 50;
            shared_data.blinking_rate       = STATE_LED_NO_BLINK;
            break;

        default:
            shared_data.pwm_state_led_red   = 0;
            shared_data.pwm_state_led_green = 0;
            shared_data.pwm_state_led_blue  = 0;
            shared_data.blinking_rate       = STATE_LED_NO_BLINK;
            break;
    }

    shared_data.state_led_data_changed = true;
}
