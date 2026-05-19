#include "main.h"
#include "logger.h"
#include "dwt.h"
#include "board.h"
#include "app.h"
#include "task_menu_attribute.h"
#include "task_menu_interface.h"
#include "display.h"

#define G_TASK_MEN_CNT_INI          0ul
#define G_TASK_MEN_TICK_CNT_INI     0ul
#define DEL_MEN_XX_MIN              0ul

/* Límites de configuración (Ej: 0% a 100%) */
const uint32_t MAX_VAL[PARAM_QTY] = {100, 100, 50, 100, 100};
const uint32_t MIN_VAL[PARAM_QTY] = {0,   0,   0,  0,   0};

/* Nombres para mostrar en el LCD */
const char *param_names[PARAM_QTY] = {"Hum. Suelo", "Hum. Amb.", "Temp. Amb.", "Luz", "Nivel Agua"};
const char *test_names[TEST_QTY] = {"Test Agua", "Test Bomba", "Test Luz", "Test LED", "Test H.Suelo", "Test Temp", "Test Buzzer"};

/* Variables internas locales para simular la memoria (EEPROM a futuro) */
uint32_t mock_config_values[PARAM_QTY] = {40, 50, 25, 60, 20}; // Valores por defecto

task_menu_dta_t task_menu_dta_list[] = {
    {DEL_MEN_XX_MIN, ST_SYS_00, EV_SYS_BTN_ESC, false, PARAM_HUM_SUELO, 0, TEST_NIVEL_AGUA},
    {DEL_MEN_XX_MIN, ST_SYS_00, EV_SYS_BTN_ESC, false, PARAM_HUM_SUELO, 0, TEST_NIVEL_AGUA},
    {DEL_MEN_XX_MIN, ST_SYS_00, EV_SYS_BTN_ESC, false, PARAM_HUM_SUELO, 0, TEST_NIVEL_AGUA},
    {DEL_MEN_XX_MIN, ST_SYS_00, EV_SYS_BTN_ESC, false, PARAM_HUM_SUELO, 0, TEST_NIVEL_AGUA}
};

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

    for (index = 0; MENU_DTA_QTY > index; index++) {
        p_task_menu_dta = &task_menu_dta_list[index];
        p_task_menu_dta->state = ST_SYS_00;
        p_task_menu_dta->event = EV_SYS_BTN_ESC;
        p_task_menu_dta->flag = false;
        p_task_menu_dta->current_parameter = PARAM_HUM_SUELO;
        p_task_menu_dta->current_test = TEST_NIVEL_AGUA;
    }

    displayInit(DISPLAY_CONNECTION_GPIO_4BITS);
    LCD_show("     SMARTCETA    ", "    Iniciando...  ");
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
        if (p_task_menu_dta->event == EV_SYS_BTN_RIGHT || p_task_menu_dta->event == EV_SYS_NEXT_TIMER) {
            p_task_menu_dta->state = (p_task_menu_dta->state == ST_SYS_04) ? ST_SYS_00 : p_task_menu_dta->state + 1;
        }
        else if (p_task_menu_dta->event == EV_SYS_BTN_LEFT) {
            p_task_menu_dta->state = (p_task_menu_dta->state == ST_SYS_00) ? ST_SYS_04 : p_task_menu_dta->state - 1;
        }
        // Ingreso a Setup
        else if (p_task_menu_dta->event == EV_SYS_BTN_ENTER) {
            active_system = SYS_SETUP;
            p_task_menu_dta->state = ST_SYS_00; // Reiniciar estado de setup
            p_task_menu_dta->current_parameter = PARAM_HUM_SUELO;
            LCD_show("Configurar:", param_names[p_task_menu_dta->current_parameter]);
            return;
        }
        // Ingreso a Test
        else if (p_task_menu_dta->event == EV_SYS_BTN_ESC_HOLD) {
            active_system = SYS_TEST;
            p_task_menu_dta->state = ST_SYS_00;
            p_task_menu_dta->current_test = TEST_NIVEL_AGUA;
            LCD_show("Modo Test:", test_names[p_task_menu_dta->current_test]);
            return;
        }

        // Actualizar pantalla según el estado actual
        // TODO: Leer los valores reales desde la estructura shared_data
        snprintf(second_row, sizeof(second_row), "Valor actual: --");
        LCD_show(param_names[p_task_menu_dta->state], second_row);
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
                p_task_menu_dta->current_parameter = (p_task_menu_dta->current_parameter + 1) % PARAM_QTY;
                LCD_show("Configurar:", param_names[p_task_menu_dta->current_parameter]);
            } else if (p_task_menu_dta->event == EV_SYS_BTN_LEFT) {
                p_task_menu_dta->current_parameter = (p_task_menu_dta->current_parameter == 0) ? (PARAM_QTY - 1) : (p_task_menu_dta->current_parameter - 1);
                LCD_show("Configurar:", param_names[p_task_menu_dta->current_parameter]);
            } else if (p_task_menu_dta->event == EV_SYS_BTN_ENTER) {
                p_task_menu_dta->state = ST_SYS_01;
                p_task_menu_dta->current_value = get_value(p_task_menu_dta->current_parameter);
                snprintf(second_row, sizeof(second_row), "> %lu", p_task_menu_dta->current_value);
                LCD_show("Nuevo Valor:", second_row);
            } else if (p_task_menu_dta->event == EV_SYS_BTN_ESC) {
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
                if (p_task_menu_dta->current_value < MAX_VAL[p_task_menu_dta->current_parameter]) {
                    p_task_menu_dta->current_value++;
                }
                snprintf(second_row, sizeof(second_row), "> %lu", p_task_menu_dta->current_value);
                LCD_show("Nuevo Valor:", second_row);
            } else if (p_task_menu_dta->event == EV_SYS_BTN_LEFT) {
                if (p_task_menu_dta->current_value > MIN_VAL[p_task_menu_dta->current_parameter]) {
                    p_task_menu_dta->current_value--;
                }
                snprintf(second_row, sizeof(second_row), "> %lu", p_task_menu_dta->current_value);
                LCD_show("Nuevo Valor:", second_row);
            } else if (p_task_menu_dta->event == EV_SYS_BTN_ENTER) {
                set_value(p_task_menu_dta->current_parameter, p_task_menu_dta->current_value);
                p_task_menu_dta->state = ST_SYS_00;
                LCD_show("Guardado!", param_names[p_task_menu_dta->current_parameter]);
            } else if (p_task_menu_dta->event == EV_SYS_BTN_ESC) {
                p_task_menu_dta->state = ST_SYS_00;
                LCD_show("Configurar:", param_names[p_task_menu_dta->current_parameter]);
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
            p_task_menu_dta->current_test = (p_task_menu_dta->current_test + 1) % TEST_QTY;
            LCD_show("Modo Test:", test_names[p_task_menu_dta->current_test]);
        } else if (p_task_menu_dta->event == EV_SYS_BTN_LEFT) {
            p_task_menu_dta->current_test = (p_task_menu_dta->current_test == 0) ? (TEST_QTY - 1) : (p_task_menu_dta->current_test - 1);
            LCD_show("Modo Test:", test_names[p_task_menu_dta->current_test]);
        } else if (p_task_menu_dta->event == EV_SYS_BTN_ENTER) {
            LCD_show("Testeando...", test_names[p_task_menu_dta->current_test]);
            test_function(p_task_menu_dta->current_test);
            LCD_show("Test Finalizado", test_names[p_task_menu_dta->current_test]);
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
    // TODO: Enlazar con lectura real de shared_data
    return mock_config_values[parameter];
}

void set_value(task_menu_parameters_t parameter, uint32_t value) {
    // TODO: Enlazar con escritura real en shared_data
    mock_config_values[parameter] = value;
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
