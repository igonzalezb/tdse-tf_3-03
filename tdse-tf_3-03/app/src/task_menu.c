#include "main.h"
#include "logger.h"
#include "dwt.h"
#include "board.h"
#include "app.h"
#include "task_menu_attribute.h"
#include "task_menu_interface.h"
#include "task_actuator_interface.h"
#include "display.h"
#include "task_system_failure.h"
#include "task_display.h"
#include "task_pump.h"

#include "stm32f1xx_hal.h"

#define FLASH_CONFIG_ADDRESS 0x0801FC00 // Página 127 (1KB)
#define PAGE_SIZE_BYTES      1024

// Cada registro ahora son 2 Half-Words (4 bytes en total): [Index][Value]
#define SLOT_SIZE_BYTES      4
#define MAX_SLOTS            (PAGE_SIZE_BYTES / SLOT_SIZE_BYTES) // 256 slots

#define G_TASK_MEN_CNT_INI          0ul
#define G_TASK_MEN_TICK_CNT_INI     0ul
#define DEL_MEN_XX_MIN              0ul
#define AUTO_SCROLL_DELAY 			5000
#define PUMP_CHECK_DELAY 			40000
#define LIGHT_CHECK_DELAY 			20000
#define DISPLAY_REFRESH_DELAY  		1000 // Refresca los valores en pantalla cada 1s

#define HUMIDITY_HYSTERESIS  10

static uint32_t last_scroll_tick;
static uint32_t last_pump_tick;
static uint32_t last_light_tick;
static uint32_t last_refresh_tick = 0;
static uint32_t last_test_tick = 0;

// para el modo falla
static system_failure_type current_display_fault = FAULT_NONE;

/* Nombres para mostrar en el LCD */
const char *param_names[PARAM_QTY];
const char *test_names[TEST_QTY];
const char *config_names[TEST_QTY];

/* Límites de configuración (Ej: 0% a 100%) */
uint32_t MAX_VAL[CONFIG_QTY];
uint32_t MIN_VAL[CONFIG_QTY];

bool testing = false;


task_menu_dta_t task_menu_dta =
		{ DEL_MEN_XX_MIN, ST_SYS_00, EV_SYS_BTN_ESC, false, PARAM_HUM_SUELO, 0, TEST_WATER_LEVEL, CONFIG_SOUNDS};

void task_menu_statechart_normal(void);
void task_menu_statechart_setup(void);
void task_menu_statechart_failure(void);
void task_menu_statechart_test(void);
char* get_sensor_value(task_menu_parameters_t parameter);
void config_load_from_flash(void);
void config_save_element_to_flash(uint16_t param_index, uint16_t value);
void test_function(task_menu_test_t current_test);

uint32_t g_task_menu_cnt;

volatile uint32_t g_task_menu_tick_cnt;

void task_menu_init(void *parameters) {
	//uint32_t index;
	//task_menu_dta_t *p_task_menu_dta;
	//shared_data_type *shared_data = (shared_data_type *) parameters;
	g_task_menu_cnt = G_TASK_MEN_CNT_INI;

	init_queue_event_task_menu();
	config_load_from_flash();

//	for (index = 0; MENU_DTA_QTY > index; index++) {
//		p_task_menu_dta = &task_menu_dta_list[index];
//		p_task_menu_dta->state = ST_SYS_00;
//		p_task_menu_dta->event = EV_SYS_BTN_ESC;
//		p_task_menu_dta->flag = false;
//		p_task_menu_dta->current_parameter = PARAM_HUM_SUELO;
//		p_task_menu_dta->current_value = 0;
//		p_task_menu_dta->current_test = TEST_WATER_LEVEL;
//	}
	shared_data.pump_on = false;
	shared_data.led_strip_on = false;
	testing = false;
	shared_data.active_system = SYS_NORMAL;

	/* Parámetros principales (Línea 1 del LCD - Máx 16 chars) */
	param_names[PARAM_HUM_SUELO] = "Humedad Suelo";   // 13 chars
	param_names[PARAM_HUM_AMB]   = "Humedad Amb.";    // 12 chars
	param_names[PARAM_TEMP_AMB]  = "Temp. Ambiente";  // 14 chars
	param_names[PARAM_LUZ]       = "Nivel de Luz";    // 12 chars
	param_names[PARAM_AGUA]      = "Nivel de Agua";   // 13 chars

	/* Menú de Pruebas (Línea 2 en Modo Test - Máx 16 chars) */
	test_names[TEST_WATER_LEVEL] = "Test Nivel Agua"; // 15 chars
	test_names[TEST_LIGHT_SENSOR]= "Test Sensor Luz"; // 15 chars
	test_names[TEST_HUMIDITY]    = "Test Hum. Suelo"; // 15 chars
	test_names[TEST_DHT22]       = "Test Temp/Hum";   // 13 chars
	test_names[TEST_STATE_LED]   = "Test LED Estado"; // 15 chars
	test_names[TEST_BUZZER]      = "Test Buzzer";     // 11 chars
	test_names[TEST_PUMP]        = "Test Bomba Agua"; // 15 chars
	test_names[TEST_LED_STRIP]   = "Test Tira LED";   // 13 chars

	/* Menú de Configuración (Línea 2 en Configurar - Máx 16 chars) */
	config_names[CONFIG_SOUNDS]     = "Sonido / Alerta"; // 15 chars
	config_names[CONFIG_LIGHT]      = "Umbral de Luz";   // 13 chars
	config_names[CONFIG_WATER_LEVEL]= "Min. Nivel Agua"; // 15 chars
	config_names[CONFIG_HUMIDITY]   = "Min. Hum. Suelo"; // 15 chars
	config_names[CONFIG_LED_STATE]  = "LED de Estado";   // 13 chars

	//TODO: cambiar algunos parametros para que sea poco-medio-mucho

	MAX_VAL[CONFIG_LIGHT] = 100;
	MAX_VAL[CONFIG_SOUNDS] = 1;
	MAX_VAL[CONFIG_WATER_LEVEL] = 100;
	MAX_VAL[CONFIG_HUMIDITY] = 100;
	MAX_VAL[CONFIG_LED_STATE] = 1;
	MIN_VAL[CONFIG_LIGHT] = 0;
	MIN_VAL[CONFIG_SOUNDS] = 0;
	MIN_VAL[CONFIG_WATER_LEVEL] = 15;
	MIN_VAL[CONFIG_HUMIDITY] = 0;
	MIN_VAL[CONFIG_LED_STATE] = 0;

	last_scroll_tick = HAL_GetTick();
	last_pump_tick = last_scroll_tick;
	last_light_tick = last_pump_tick;

	// Inicializo el buzzer y el Led de estados en modo normal.
	shared_data.config_values[CONFIG_SOUNDS] ? put_event_task_actuator(ID_ACT_BUZZER, EV_BUZZER_1PULSE) : 0;
	shared_data.config_values[CONFIG_LED_STATE] ? put_event_task_actuator(ID_ACT_STATE_LED, EV_STATE_LED_SYS_NORMAL) : 0;

}

void task_menu_update(void *parameters) {
	//shared_data_type *shared_data = (shared_data_type *) parameters;
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
	task_menu_dta_t *p_task_menu_dta = &task_menu_dta;

	if (true == any_event_task_menu()) {
		p_task_menu_dta->flag = true;
		p_task_menu_dta->event = get_event_task_menu();
	}

	if (p_task_menu_dta->flag) {
		p_task_menu_dta->flag = false;

		/*if (p_task_menu_dta->event == EV_SYS_FAILURE){
			LOGGER_INFO("SYS FAILURE");
			shared_data.active_system = SYS_FAILURE;
			p_task_menu_dta->state = ST_SYS_00;
			shared_data.config_values[CONFIG_SOUNDS] ? put_event_task_actuator(ID_ACT_BUZZER, EV_BUZZER_INTERMITTENT) : 0;
			shared_data.config_values[CONFIG_LED_STATE] ? put_event_task_actuator(ID_ACT_STATE_LED, EV_STATE_LED_SYS_FAILURE) : 0;

		}
		// Navegación entre botones (derecha/izquierda)
		else */if (p_task_menu_dta->event == EV_SYS_BTN_RIGHT) {
			LOGGER_INFO("BTN_RIGHT PRESSED");
			p_task_menu_dta->current_parameter =
					(p_task_menu_dta->current_parameter == (PARAM_QTY-1)) ?
							0 : p_task_menu_dta->current_parameter + 1;
		} else if (p_task_menu_dta->event == EV_SYS_BTN_LEFT) {
			LOGGER_INFO("BTN_LEFT PRESSED");
			p_task_menu_dta->current_parameter =
					(p_task_menu_dta->current_parameter == 0) ?
							(PARAM_QTY-1) : p_task_menu_dta->current_parameter - 1;
		}
		// Ingreso a Setup
		else if (p_task_menu_dta->event == EV_SYS_BTN_ENTER) {
			LOGGER_INFO("BTN_ENTER PRESSED");
			shared_data.active_system = SYS_SETUP;
			shared_data.config_values[CONFIG_SOUNDS] ? put_event_task_actuator(ID_ACT_BUZZER, EV_BUZZER_1PULSE) : 0;
			shared_data.config_values[CONFIG_LED_STATE] ? put_event_task_actuator(ID_ACT_STATE_LED, EV_STATE_LED_SYS_SETUP) : 0;
			p_task_menu_dta->current_parameter = 0;
			LCD_show("Configurar:",	config_names[0]);
			return;
		}
		// Ingreso a Test
		else if (p_task_menu_dta->event == EV_SYS_BTN_ESC_HOLD) {
			LOGGER_INFO("BTN_ESC HOLD");
			shared_data.active_system = SYS_TEST;
			shared_data.config_values[CONFIG_SOUNDS] ? put_event_task_actuator(ID_ACT_BUZZER, EV_BUZZER_1PULSE) : 0;
			shared_data.config_values[CONFIG_LED_STATE] ? put_event_task_actuator(ID_ACT_STATE_LED, EV_STATE_LED_SYS_TEST) : 0;
			p_task_menu_dta->current_parameter = 0;
			LCD_show("Modo Test:", test_names[0]);
			return;
		}
		last_scroll_tick = HAL_GetTick();

		LCD_show(param_names[p_task_menu_dta->current_parameter], get_sensor_value(p_task_menu_dta->current_parameter), CENTER);
	}
	/*=============== AUTOSCROLL =======================================*/
	else if ((HAL_GetTick() - last_scroll_tick) >= AUTO_SCROLL_DELAY) {
		p_task_menu_dta->current_parameter =
				(p_task_menu_dta->current_parameter == (PARAM_QTY-1)) ?
						0 : p_task_menu_dta->current_parameter + 1;
		LCD_show(param_names[p_task_menu_dta->current_parameter], get_sensor_value(p_task_menu_dta->current_parameter), CENTER);
		last_scroll_tick = HAL_GetTick();
	}
	/*=============== REFRESCO PERIÓDICO DEL SENSOR =====================*/
	else if ((HAL_GetTick() - last_refresh_tick) >= DISPLAY_REFRESH_DELAY) {
			// Actualiza únicamente el número/valor del sensor actual sin cambiar de pantalla
			LCD_show(param_names[p_task_menu_dta->current_parameter], get_sensor_value(p_task_menu_dta->current_parameter), CENTER);
			last_refresh_tick = HAL_GetTick();
	}
	/*=============== LED STRIP =======================================*/
	if ((HAL_GetTick() - last_light_tick) >= LIGHT_CHECK_DELAY) {
		// todo si está prendido que no vuelva a mandar el evento de prender
		if (shared_data.light_percent <= shared_data.config_values[CONFIG_LIGHT])
		{
			LOGGER_INFO("ACTIVO LED STRIP");
			put_event_task_actuator(ID_ACT_LED_STRIP, EV_LED_STRIP_ON);
		}
		else if ((shared_data.light_percent) >= (shared_data.config_values[CONFIG_LIGHT] + 10))
		{
			LOGGER_INFO("DESACTIVO LED STRIP");
			put_event_task_actuator(ID_ACT_LED_STRIP, EV_LED_STRIP_OFF);
		}
		last_light_tick = HAL_GetTick();
	}

	/*=============== PUMP =======================================*/
/*	else if (((HAL_GetTick() - last_pump_tick) >= PUMP_CHECK_DELAY) || shared_data.pump_on) {
		if (shared_data.pump_on && shared_data.humidity_percent < (shared_data.config_values[CONFIG_HUMIDITY]+10)){
			put_event_task_actuator(ID_ACT_PUMP, EV_PUMP_OFF);
			shared_data.config_values[CONFIG_SOUNDS] ? put_event_task_actuator(ID_ACT_BUZZER, EV_BUZZER_2PULSE) : 0;
			shared_data.config_values[CONFIG_LED_STATE] ? put_event_task_actuator(ID_ACT_STATE_LED, EV_STATE_LED_SYS_NORMAL) : 0;
		}
		else if (shared_data.water_level_percent >= shared_data.config_values[CONFIG_WATER_LEVEL]
				&& shared_data.humidity_percent < shared_data.config_values[CONFIG_HUMIDITY])
		{
			put_event_task_actuator(ID_ACT_PUMP, EV_PUMP_ON);
			shared_data.config_values[CONFIG_SOUNDS] ? put_event_task_actuator(ID_ACT_BUZZER, EV_BUZZER_2PULSE) : 0;
			shared_data.config_values[CONFIG_LED_STATE] ? put_event_task_actuator(ID_ACT_STATE_LED, EV_STATE_LED_WATER) : 0;
		}

		last_pump_tick = HAL_GetTick();
	}*/
	if (((HAL_GetTick() - last_pump_tick) >= PUMP_CHECK_DELAY) || (get_pump_state() != ST_PUMP_IDLE)) {

	    // 1. Calcular objetivo con techo de seguridad al 100%
	    uint16_t target_humidity = shared_data.config_values[CONFIG_HUMIDITY] + HUMIDITY_HYSTERESIS;
	    if (target_humidity > 100) {
	        target_humidity = 100;
	    }

	    // 2. APAGAR BOMBA:
	    // Si la bomba está encendida Y (alcanzó la humedad deseada O se quedó sin agua)
	   if ((get_pump_state() == ST_PUMP_ON) &&
		   (shared_data.humidity_percent >= target_humidity ||
			shared_data.water_level_percent < shared_data.config_values[CONFIG_WATER_LEVEL]))
		{
	    	LOGGER_INFO("DESACTIVO BOMBA");
	        put_event_task_actuator(ID_ACT_PUMP, EV_PUMP_OFF);
	        shared_data.config_values[CONFIG_SOUNDS] ? put_event_task_actuator(ID_ACT_BUZZER, EV_BUZZER_2PULSE) : 0;
	        shared_data.config_values[CONFIG_LED_STATE] ? put_event_task_actuator(ID_ACT_STATE_LED, EV_STATE_LED_SYS_NORMAL) : 0;

	        last_pump_tick = HAL_GetTick();
	    }
	    // 3. PRENDER BOMBA:
	    // Si la bomba está apagada Y hay agua suficiente Y la humedad cayó por debajo del mínimo
	   else if ((get_pump_state() == ST_PUMP_IDLE) &&
			shared_data.water_level_percent >= shared_data.config_values[CONFIG_WATER_LEVEL] &&
			shared_data.humidity_percent < shared_data.config_values[CONFIG_HUMIDITY])
	    {
	    	LOGGER_INFO("ACTIVO BOMBA");
	        put_event_task_actuator(ID_ACT_PUMP, EV_PUMP_ON);
	        shared_data.config_values[CONFIG_SOUNDS] ? put_event_task_actuator(ID_ACT_BUZZER, EV_BUZZER_2PULSE) : 0;
	        shared_data.config_values[CONFIG_LED_STATE] ? put_event_task_actuator(ID_ACT_STATE_LED, EV_STATE_LED_WATER) : 0;

	        last_pump_tick = HAL_GetTick();
	    }
	}
}

void task_menu_statechart_setup(void) {
	task_menu_dta_t *p_task_menu_dta = &task_menu_dta;
	char second_row[20];

	if (true == any_event_task_menu()) {
		p_task_menu_dta->flag = true;
		p_task_menu_dta->event = get_event_task_menu();
	}

	switch (p_task_menu_dta->state) {
	case ST_SYS_00: // Selección de parámetro a configurar
		if (p_task_menu_dta->flag) {
			p_task_menu_dta->flag = false;
			/*if (p_task_menu_dta->event == EV_SYS_FAILURE){
				LOGGER_INFO("SYS FAILURE");
				shared_data.active_system = SYS_FAILURE;
				p_task_menu_dta->state = ST_SYS_00;
				shared_data.config_values[CONFIG_SOUNDS] ? put_event_task_actuator(ID_ACT_BUZZER, EV_BUZZER_INTERMITTENT) : 0;
				shared_data.config_values[CONFIG_LED_STATE] ? put_event_task_actuator(ID_ACT_STATE_LED, EV_STATE_LED_SYS_FAILURE) : 0;

			}

			else */if (p_task_menu_dta->event == EV_SYS_BTN_RIGHT) {
				LOGGER_INFO("BTN_RIGHT PRESSED");
				p_task_menu_dta->current_config =
						(p_task_menu_dta->current_config == (CONFIG_QTY - 1)) ?
								0 : p_task_menu_dta->current_config + 1;
				LCD_show("Configurar:",
						config_names[p_task_menu_dta->current_config]);
			} else if (p_task_menu_dta->event == EV_SYS_BTN_LEFT) {
				LOGGER_INFO("BTN_LEFT PRESSED");
				p_task_menu_dta->current_config =
						(p_task_menu_dta->current_config == 0) ?
								(CONFIG_QTY - 1) :
								(p_task_menu_dta->current_config - 1);
				LCD_show("Configurar:",
						config_names[p_task_menu_dta->current_config]);
			} else if (p_task_menu_dta->event == EV_SYS_BTN_ENTER) {
				LOGGER_INFO("BTN_ENTER PRESSED");
				p_task_menu_dta->state = ST_SYS_01;
				if (p_task_menu_dta->current_config == CONFIG_SOUNDS || p_task_menu_dta->current_config == CONFIG_LED_STATE)
				{
					LCD_show(config_names[p_task_menu_dta->current_config], shared_data.config_values[p_task_menu_dta->current_config] ? "> ON" : "> OFF");
				}
				else
				{
					snprintf(second_row, sizeof(second_row), "> %u", shared_data.config_values[p_task_menu_dta->current_config]);
					LCD_show(config_names[p_task_menu_dta->current_config], second_row);
				}
			} else if (p_task_menu_dta->event == EV_SYS_BTN_ESC) {
				LOGGER_INFO("BTN_ESC PRESSED");
				shared_data.active_system = SYS_NORMAL;
				p_task_menu_dta->state = ST_SYS_00;
				p_task_menu_dta->current_config = 0;
				shared_data.config_values[CONFIG_SOUNDS] ? put_event_task_actuator(ID_ACT_BUZZER, EV_BUZZER_1PULSE) : 0;
				shared_data.config_values[CONFIG_LED_STATE] ? put_event_task_actuator(ID_ACT_STATE_LED, EV_STATE_LED_SYS_NORMAL) : 0;
				LCD_show("Saliendo...", "");
			}
		}
		break;

	case ST_SYS_01: // Modificación del valor del parámetro
		if (p_task_menu_dta->flag) {
			p_task_menu_dta->flag = false;
			/*if (p_task_menu_dta->event == EV_SYS_FAILURE){
				LOGGER_INFO("SYS FAILURE");
				shared_data.active_system = SYS_FAILURE;
				p_task_menu_dta->state = ST_SYS_00;
				shared_data.config_values[CONFIG_SOUNDS] ? put_event_task_actuator(ID_ACT_BUZZER, EV_BUZZER_INTERMITTENT) : 0;
				shared_data.config_values[CONFIG_LED_STATE] ? put_event_task_actuator(ID_ACT_STATE_LED, EV_STATE_LED_SYS_FAILURE) : 0;

			}
			else*/ if (p_task_menu_dta->event == EV_SYS_BTN_RIGHT) {
				LOGGER_INFO("BTN_RIGHT PRESSED");

				(shared_data.config_values[p_task_menu_dta->current_config]
						== MAX_VAL[p_task_menu_dta->current_config]) ? shared_data.config_values[p_task_menu_dta->current_config] = 0:
								shared_data.config_values[p_task_menu_dta->current_config]++;
				if (p_task_menu_dta->current_config == CONFIG_SOUNDS || p_task_menu_dta->current_config == CONFIG_LED_STATE)
				{
					LCD_show(config_names[p_task_menu_dta->current_config], shared_data.config_values[p_task_menu_dta->current_config] ? "> ON" : "> OFF");
				}
				else
				{
					snprintf(second_row, sizeof(second_row), "> %u", shared_data.config_values[p_task_menu_dta->current_config]);
					LCD_show(config_names[p_task_menu_dta->current_config], second_row);
				}

			} else if (p_task_menu_dta->event == EV_SYS_BTN_LEFT) {
				LOGGER_INFO("BTN_LEFT PRESSED");
				(shared_data.config_values[p_task_menu_dta->current_config] == MIN_VAL[p_task_menu_dta->current_config]) ?
						shared_data.config_values[p_task_menu_dta->current_config] = MAX_VAL[p_task_menu_dta->current_config]:
						shared_data.config_values[p_task_menu_dta->current_config]--;
				if (p_task_menu_dta->current_config == CONFIG_SOUNDS || p_task_menu_dta->current_config == CONFIG_LED_STATE)
				{
					LCD_show(config_names[p_task_menu_dta->current_config], shared_data.config_values[p_task_menu_dta->current_config] ? "> ON" : "> OFF");
				}
				else
				{
					snprintf(second_row, sizeof(second_row), "> %u", shared_data.config_values[p_task_menu_dta->current_config]);
					LCD_show(config_names[p_task_menu_dta->current_config], second_row);
				}
			} else if (p_task_menu_dta->event == EV_SYS_BTN_ENTER) {
				LOGGER_INFO("BTN_ENTER PRESSED");

				config_save_element_to_flash(p_task_menu_dta->current_config,
						shared_data.config_values[p_task_menu_dta->current_config]);
				p_task_menu_dta->state = ST_SYS_00;
				LCD_show("Guardado!",
						config_names[p_task_menu_dta->current_config]);
				shared_data.config_values[CONFIG_SOUNDS] ? put_event_task_actuator(ID_ACT_BUZZER, EV_BUZZER_1PULSE) : 0;
				shared_data.config_values[CONFIG_LED_STATE] ? 0 : put_event_task_actuator(ID_ACT_STATE_LED, EV_STATE_LED_OFF);

			} else if (p_task_menu_dta->event == EV_SYS_BTN_ESC) {
				LOGGER_INFO("BTN_ESC PRESSED");
				p_task_menu_dta->state = ST_SYS_00;
				LCD_show("Configurar:",
						config_names[p_task_menu_dta->current_config]);
			}
		}
		break;
	default:
		break;
	}
}

void task_menu_statechart_test(void) {
	task_menu_dta_t *p_task_menu_dta = &task_menu_dta;
	char segunda_linea[17];

	if (true == any_event_task_menu()) {
		p_task_menu_dta->flag = true;
		p_task_menu_dta->event = get_event_task_menu();
	}
	else if (testing) {
			if ((HAL_GetTick() - last_test_tick) >= DISPLAY_REFRESH_DELAY) {

				switch (p_task_menu_dta->current_test) {
					case TEST_WATER_LEVEL:
						snprintf(segunda_linea, sizeof(segunda_linea), "Niv. Agua: %s", get_sensor_value(PARAM_AGUA));
						LCD_show("Testeando...", segunda_linea);
						break;
					case TEST_LIGHT_SENSOR:
						snprintf(segunda_linea, sizeof(segunda_linea), "Sens. Luz: %s", get_sensor_value(PARAM_LUZ));
						LCD_show("Testeando...", segunda_linea);
						break;
					case TEST_HUMIDITY:
						snprintf(segunda_linea, sizeof(segunda_linea), "Hum. S: %s", get_sensor_value(PARAM_HUM_SUELO));
						LCD_show("Testeando...", segunda_linea);
						break;
					case TEST_DHT22:
						snprintf(segunda_linea, sizeof(segunda_linea), "DHT22: %s %s", get_sensor_value(PARAM_HUM_AMB), get_sensor_value(PARAM_TEMP_AMB));
						LCD_show("Testeando...", segunda_linea);
						break;
					case TEST_STATE_LED:
					case TEST_BUZZER:
					case TEST_PUMP:
					case TEST_LED_STRIP:
						break;
					default:
						break;
				}
				last_test_tick = HAL_GetTick(); // Reiniciamos el temporizador
			}
		}

	if (p_task_menu_dta->flag)
	{
		p_task_menu_dta->flag = false;
		/*if (p_task_menu_dta->event == EV_SYS_FAILURE){
			LOGGER_INFO("SYS FAILURE");
			p_task_menu_dta->state = ST_SYS_00;
			shared_data.active_system = SYS_FAILURE;
			shared_data.config_values[CONFIG_SOUNDS] ? put_event_task_actuator(ID_ACT_BUZZER, EV_BUZZER_INTERMITTENT) : 0;
			shared_data.config_values[CONFIG_LED_STATE] ? put_event_task_actuator(ID_ACT_STATE_LED, EV_STATE_LED_SYS_FAILURE) : 0;

		}
		else*/ if (p_task_menu_dta->event == EV_SYS_BTN_RIGHT) {
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
			testing = true;
			last_test_tick = HAL_GetTick();
			switch (p_task_menu_dta->current_test) {
				case TEST_WATER_LEVEL:
					snprintf(segunda_linea, sizeof(segunda_linea), "Niv. Agua: %s", get_sensor_value(PARAM_AGUA));
					LCD_show("Testeando...", segunda_linea);
					break;
				case TEST_LIGHT_SENSOR:
					snprintf(segunda_linea, sizeof(segunda_linea), "Sens. Luz: %s", get_sensor_value(PARAM_LUZ));
					LCD_show("Testeando...", segunda_linea);
					break;
				case TEST_HUMIDITY:
					snprintf(segunda_linea, sizeof(segunda_linea), "Hum. S: %s", get_sensor_value(PARAM_HUM_SUELO));
					LCD_show("Testeando...", segunda_linea);
					break;
				case TEST_DHT22:
					snprintf(segunda_linea, sizeof(segunda_linea), "DHT22: %s %s", get_sensor_value(PARAM_HUM_AMB), get_sensor_value(PARAM_TEMP_AMB));
					LCD_show("Testeando...", segunda_linea);
					break;
				case TEST_STATE_LED:
					put_event_task_actuator(ID_ACT_STATE_LED, EV_STATE_LED_ON);
					LCD_show("Testeando...", test_names[p_task_menu_dta->current_test]);
					break;
				case TEST_BUZZER:
					put_event_task_actuator(ID_ACT_BUZZER, EV_BUZZER_ON);
					LCD_show("Testeando...", test_names[p_task_menu_dta->current_test]);
					break;
				case TEST_PUMP:
					put_event_task_actuator(ID_ACT_PUMP, EV_PUMP_ON);
					LCD_show("Testeando...", test_names[p_task_menu_dta->current_test]);
					break;
				case TEST_LED_STRIP:
					put_event_task_actuator(ID_ACT_LED_STRIP, EV_LED_STRIP_ON);
					LCD_show("Testeando...", test_names[p_task_menu_dta->current_test]);
					break;

					break;
				default:
					break;
			}
		} else if (p_task_menu_dta->event == EV_SYS_BTN_ESC && testing) {

			switch (p_task_menu_dta->current_test) {
				case TEST_STATE_LED:
					put_event_task_actuator(ID_ACT_STATE_LED, EV_STATE_LED_OFF);
					break;
				case TEST_BUZZER:
					put_event_task_actuator(ID_ACT_BUZZER, EV_BUZZER_OFF);
					break;
				case TEST_PUMP:
					put_event_task_actuator(ID_ACT_PUMP, EV_PUMP_OFF);
					break;
				case TEST_LED_STRIP:
					put_event_task_actuator(ID_ACT_LED_STRIP, EV_LED_STRIP_OFF);
					break;

					break;
				default:
					break;
			}
			testing = false;
			LCD_show("Modo Test:", test_names[p_task_menu_dta->current_test]);
		} else if (p_task_menu_dta->event == EV_SYS_BTN_ESC) {
			shared_data.active_system = SYS_NORMAL;
			p_task_menu_dta->state = ST_SYS_00; // Forzar refresco
			shared_data.config_values[CONFIG_SOUNDS] ? put_event_task_actuator(ID_ACT_BUZZER, EV_BUZZER_1PULSE) : 0;
			shared_data.config_values[CONFIG_LED_STATE] ? put_event_task_actuator(ID_ACT_STATE_LED, EV_STATE_LED_SYS_NORMAL) : 0;
			LCD_show("Saliendo...", "");
		}
	}
}

void task_menu_statechart_failure(void){
    task_menu_dta_t *p_task_menu_dta = &task_menu_dta;
    // Para no llamar a LCD_Show() innecesariamente
    bool update_display = false;

    // Estados del modo falla
    bool is_locked = task_system_failure_is_locked();
    bool ready_to_restore = task_system_failure_can_restore();

    switch (p_task_menu_dta->state) {
    	// Apagar bomba y tira LED la primera vez, para no obturar de eventos al actuador
		case ST_SYS_00:
			put_event_task_actuator(ID_ACT_PUMP, EV_PUMP_OFF);
			put_event_task_actuator(ID_ACT_LED_STRIP, EV_LED_STRIP_OFF);
			shared_data.pump_on = false;
			shared_data.led_strip_on = false;

			if (is_locked) {shared_data.config_values[CONFIG_LED_STATE] ? put_event_task_actuator(ID_ACT_STATE_LED, EV_STATE_LED_SYS_LOCKED) : 0;}

			current_display_fault = task_system_failure_get_valid_fault(FAULT_NONE);
			last_scroll_tick = HAL_GetTick();
			update_display = true;

			p_task_menu_dta->state = ST_SYS_01; // ST 01 = procesamiento de fallas y pantallas LCD
			break;

		case ST_SYS_01:
			// Validar que la falla en pantalla siga existiendo
			system_failure_type valid_fault = task_system_failure_get_valid_fault(current_display_fault);

			if (valid_fault != current_display_fault) {
			    current_display_fault = valid_fault;
			    update_display = true;
			}

			// Leer eventos pendientes
			if (true == any_event_task_menu()) {
				p_task_menu_dta->flag = true;
				p_task_menu_dta->event = get_event_task_menu();
			}

			// Procesar Botones
			if (p_task_menu_dta->flag) {
				p_task_menu_dta->flag = false;

				if (ready_to_restore && p_task_menu_dta->event == EV_SYS_BTN_ESC_HOLD) {
					task_system_failure_clear_all();
					shared_data.active_system = SYS_NORMAL;
					shared_data.config_values[CONFIG_SOUNDS] ? put_event_task_actuator(ID_ACT_BUZZER, EV_BUZZER_1PULSE) : 0;
					shared_data.config_values[CONFIG_LED_STATE] ? put_event_task_actuator(ID_ACT_STATE_LED, EV_STATE_LED_SYS_NORMAL) : 0;
					put_event_task_menu(EV_SYS_BTN_ESC);
					p_task_menu_dta->state = ST_SYS_00;
					current_display_fault = FAULT_NONE;
					return;
				}

				if (!is_locked) {
					if (p_task_menu_dta->event == EV_SYS_BTN_RIGHT) {
						current_display_fault = task_system_failure_get_next(current_display_fault);
						last_scroll_tick = HAL_GetTick();
						update_display = true;
					}
					else if (p_task_menu_dta->event == EV_SYS_BTN_LEFT) {
						current_display_fault = task_system_failure_get_prev(current_display_fault);
						last_scroll_tick = HAL_GetTick();
						update_display = true;
					}
				}
			}

			// Autoscroll
			if ((HAL_GetTick() - last_scroll_tick) >= AUTO_SCROLL_DELAY) {
				last_scroll_tick = HAL_GetTick();
				update_display = true;

				if (!is_locked) {
					current_display_fault = task_system_failure_get_next(current_display_fault);
				}
			}
			break;
		case ST_SYS_02: break;
		case ST_SYS_03: break;
		case ST_SYS_04: break;
		default: break;
    }

	if (update_display) {
		if (is_locked) {
			LCD_show("SistemaBloqueado", task_system_failure_get_name(current_display_fault));
		}
		else if (current_display_fault == FAULT_RESTORE) {
			LCD_show("Para restaurar", "mantenga ESCAPE");
		}
		else {
			char row1[21];
			sprintf(row1, "FALLA CODIGO: %02d", current_display_fault);
			LCD_show(row1, task_system_failure_get_name(current_display_fault));
		}
	}
}


char* get_sensor_value(task_menu_parameters_t parameter) {
    static char value[12];

    switch (parameter) {
        case PARAM_HUM_SUELO:
            snprintf(value, sizeof(value), "%u%%", shared_data.humidity_percent);
            break;
        case PARAM_HUM_AMB:
            snprintf(value, sizeof(value), "%u%%", shared_data.dht22_humidity);
            break;
        case PARAM_LUZ:
            snprintf(value, sizeof(value), "%u%%", shared_data.light_percent);
            break;
        case PARAM_AGUA:
            snprintf(value, sizeof(value), "%u%%", shared_data.water_level_percent);
            break;
        case PARAM_TEMP_AMB:
            snprintf(value, sizeof(value), "%u C", shared_data.dht22_temperature);
            break;
        default:
            snprintf(value, sizeof(value), "---");
            break;
    }
    return value;
}

void config_load_from_flash(void) {
    // 1. Definimos los valores por defecto en una estructura/arreglo temporal
    uint16_t defaults[CONFIG_QTY];
    defaults[CONFIG_SOUNDS] = 1;
    defaults[CONFIG_LIGHT] = 50;
    defaults[CONFIG_WATER_LEVEL] = 40;
    defaults[CONFIG_HUMIDITY] = 70;
    defaults[CONFIG_LED_STATE] = 1;

    uint16_t *flash_ptr = (uint16_t *)FLASH_CONFIG_ADDRESS;

    // 2. CHEQUEO CRÍTICO: ¿El mismísimo primer slot de la Flash está vacío (0xFFFF)?
    // Si el índice del primer slot es 0xFFFF, significa que la memoria NUNCA fue escrita.
    if (flash_ptr[0] == 0xFFFF) {

        HAL_FLASH_Unlock();

        // Escribimos los 5 parámetros por defecto consecutivos para inicializar la memoria
        for (uint16_t j = 0; j < CONFIG_QTY; j++) {
            uint32_t addr = FLASH_CONFIG_ADDRESS + (j * SLOT_SIZE_BYTES);

            // Guardamos el par: [Índice, Valor]
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, j);
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr + 2, defaults[j]);

            // También los cargamos en el arreglo activo de la RAM que usa tu menú
            shared_data.config_values[j] = defaults[j];
        }

        HAL_FLASH_Lock();
        return; // Inicialización de fábrica terminada. Salimos.
    }

    // 3. CASO NORMAL: Si la Flash NO estaba vacía, procesamos el historial como antes
    // (Cargamos los defaults en RAM primero por seguridad si algún índice falta)
    for (int j = 0; j < CONFIG_QTY; j++) {
    	shared_data.config_values[j] = defaults[j];
    }

    // Recorremos los slots aplicando los cambios cronológicamente
    for (int i = 0; i < MAX_SLOTS; i++) {
        uint32_t offset = i * 2;
        uint16_t param_idx = flash_ptr[offset];
        uint16_t param_val = flash_ptr[offset + 1];

        if (param_idx == 0xFFFF) {
            break; // Fin del historial de cambios
        }

        if (param_idx < CONFIG_QTY) {
        	shared_data.config_values[param_idx] = param_val;
        }
    }
}

void config_save_element_to_flash(uint16_t param_index, uint16_t value) {
    uint16_t *flash_ptr = (uint16_t *)FLASH_CONFIG_ADDRESS;
    int target_slot_index = -1;

    // 1. Buscar el primer slot vacío (donde el índice sea 0xFFFF)
    for (int i = 0; i < MAX_SLOTS; i++) {
        if (flash_ptr[i * 2] == 0xFFFF) {
            target_slot_index = i;
            break;
        }
    }

    HAL_FLASH_Unlock();

    // 2. Si la página se llenó (256 cambios acumulados), hay que consolidar y borrar
    if (target_slot_index == -1) {
        // Guardamos el cambio actual en el arreglo antes de consolidar
    	shared_data.config_values[param_index] = value;

        FLASH_EraseInitTypeDef EraseInitStruct;
        uint32_t PageError = 0;

        EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
        EraseInitStruct.PageAddress = FLASH_CONFIG_ADDRESS;
        EraseInitStruct.NbPages = 1;

        // Borramos la página (toma ~30ms, pasa 1 vez cada 256 guardados)
        HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);

        // Al borrar la página perdimos el historial, así que reescribimos
        // el estado actual completo de los 5 parámetros para que sirva de base limpia.
        for (uint16_t j = 0; j < CONFIG_QTY; j++) {
            uint32_t addr = FLASH_CONFIG_ADDRESS + (j * SLOT_SIZE_BYTES);
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, j);
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr + 2, shared_data.config_values[j]);
        }

        HAL_FLASH_Lock();
        return;
    }

    // 3. Caso normal: Hay espacio libre. Escribimos solo el par [Index, Value]
    uint32_t base_address = FLASH_CONFIG_ADDRESS + (target_slot_index * SLOT_SIZE_BYTES);

    // Escribimos primero el valor y al final el índice (buena práctica por si se corta la luz a medias)
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, base_address + 2, value);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, base_address, param_index);

    HAL_FLASH_Lock();
}
