#include "main.h"
#include "logger.h"
#include "dwt.h"
#include "board.h"
#include "app.h"
#include "task_sensor_attribute.h"
#include "task_menu_attribute.h"
#include "task_menu_interface.h"

#define G_TASK_SEN_CNT_INIT         0ul
#define G_TASK_SEN_TICK_CNT_INI     0ul

#define DEL_BTN_XX_MIN              0ul
#define DEL_BTN_XX_MED              25ul
#define DEL_BTN_XX_MAX              50ul
#define DEL_BTN_XX_HOLD             1000ul

/* Mapeo de botones a la nueva lógica de la Smartceta */
task_sensor_cfg_t task_sensor_cfg_list[] = {
    // ENTER
    { ID_BTN_ENT, BTN_ENT_PORT, BTN_ENT_PIN, BTN_ENT_PRESSED, DEL_BTN_XX_MAX, DEL_BTN_XX_HOLD,
      EV_SYS_NONE, EV_SYS_BTN_ENTER, EV_SYS_NONE },
    // NEXT (DERECHA)
    { ID_BTN_NEX, BTN_NEX_PORT, BTN_NEX_PIN, BTN_NEX_PRESSED, DEL_BTN_XX_MAX, DEL_BTN_XX_HOLD,
      EV_SYS_NONE, EV_SYS_BTN_RIGHT, EV_SYS_NEXT_TIMER },
    // PREV (IZQUIERDA)
    { ID_BTN_PREV, BTN_PREV_PORT, BTN_PREV_PIN, BTN_PREV_PRESSED, DEL_BTN_XX_MAX, DEL_BTN_XX_HOLD,
      EV_SYS_NONE, EV_SYS_BTN_LEFT, EV_SYS_NONE },
    // ESC
    { ID_BTN_ESC, BTN_ESC_PORT, BTN_ESC_PIN, BTN_ESC_PRESSED, DEL_BTN_XX_MAX, DEL_BTN_XX_HOLD,
      EV_SYS_NONE, EV_SYS_BTN_ESC, EV_SYS_BTN_ESC_HOLD }
};

#define SENSOR_CFG_QTY  (sizeof(task_sensor_cfg_list)/sizeof(task_sensor_cfg_t))

task_sensor_dta_t task_sensor_dta_list[] = {
    { DEL_BTN_XX_MIN, DEL_BTN_XX_MIN, ST_BTN_XX_UP, EV_BTN_XX_UP },
    { DEL_BTN_XX_MIN, DEL_BTN_XX_MIN, ST_BTN_XX_UP, EV_BTN_XX_UP },
    { DEL_BTN_XX_MIN, DEL_BTN_XX_MIN, ST_BTN_XX_UP, EV_BTN_XX_UP },
    { DEL_BTN_XX_MIN, DEL_BTN_XX_MIN, ST_BTN_XX_UP, EV_BTN_XX_UP }
};

#define SENSOR_DTA_QTY  (sizeof(task_sensor_dta_list)/sizeof(task_sensor_dta_t))

void task_sensor_statechart(void);

uint32_t g_task_sensor_cnt;
volatile uint32_t g_task_sensor_tick_cnt;

void task_sensor_init(void *parameters) {
    uint32_t index;
    g_task_sensor_cnt = G_TASK_SEN_CNT_INIT;

    for (index = 0; SENSOR_DTA_QTY > index; index++) {
        task_sensor_dta_list[index].state = ST_BTN_XX_UP;
        task_sensor_dta_list[index].event = EV_BTN_XX_UP;
    }
}

void task_sensor_update(void *parameters) {
    bool b_time_update_required = false;

    __asm("CPSID i");
    if (G_TASK_SEN_TICK_CNT_INI < g_task_sensor_tick_cnt) {
        g_task_sensor_tick_cnt--;
        b_time_update_required = true;
    }
    __asm("CPSIE i");

    while (b_time_update_required) {
        g_task_sensor_cnt++;
        task_sensor_statechart();

        __asm("CPSID i");
        if (G_TASK_SEN_TICK_CNT_INI < g_task_sensor_tick_cnt) {
            g_task_sensor_tick_cnt--;
            b_time_update_required = true;
        } else {
            b_time_update_required = false;
        }
        __asm("CPSIE i");
    }
}

void task_sensor_statechart(void) {
    uint32_t index;
    const task_sensor_cfg_t *p_task_sensor_cfg;
    task_sensor_dta_t *p_task_sensor_dta;

    for (index = 0; SENSOR_DTA_QTY > index; index++) {
        p_task_sensor_cfg = &task_sensor_cfg_list[index];
        p_task_sensor_dta = &task_sensor_dta_list[index];

        if (p_task_sensor_cfg->pressed == HAL_GPIO_ReadPin(p_task_sensor_cfg->gpio_port, p_task_sensor_cfg->pin)) {
            p_task_sensor_dta->event = EV_BTN_XX_DOWN;
        } else {
            p_task_sensor_dta->event = EV_BTN_XX_UP;
        }

        switch (p_task_sensor_dta->state) {
        case ST_BTN_XX_UP:
            if (EV_BTN_XX_DOWN == p_task_sensor_dta->event) {
                p_task_sensor_dta->tick = p_task_sensor_cfg->tick_max;
                p_task_sensor_dta->state = ST_BTN_XX_FALLING;
            }
            break;

        case ST_BTN_XX_FALLING:
            if (EV_BTN_XX_UP == p_task_sensor_dta->event) {
                if (DEL_BTN_XX_MIN == p_task_sensor_dta->tick) {
                    p_task_sensor_dta->state = ST_BTN_XX_UP;
                } else {
                    p_task_sensor_dta->tick--;
                }
            }
            if (EV_BTN_XX_DOWN == p_task_sensor_dta->event) {
                if (DEL_BTN_XX_MIN == p_task_sensor_dta->tick) {
                    p_task_sensor_dta->state = ST_BTN_XX_DOWN;
                    if (p_task_sensor_cfg->signal_down != EV_SYS_NONE) {
                        put_event_task_menu(p_task_sensor_cfg->signal_down);
                    }
                    p_task_sensor_dta->tick_hold = p_task_sensor_cfg->tick_hold_max;
                } else {
                    p_task_sensor_dta->tick--;
                }
            }
            break;

        case ST_BTN_XX_DOWN:
            if (EV_BTN_XX_DOWN == p_task_sensor_dta->event) {
                if (DEL_BTN_XX_MIN == p_task_sensor_dta->tick_hold) {
                    p_task_sensor_dta->tick_hold = p_task_sensor_cfg->tick_max;
                    if (p_task_sensor_cfg->signal_hold != EV_SYS_NONE) {
                        put_event_task_menu(p_task_sensor_cfg->signal_hold);
                    }
                } else {
                    p_task_sensor_dta->tick_hold--;
                }
            }
            if (EV_BTN_XX_UP == p_task_sensor_dta->event) {
                p_task_sensor_dta->state = ST_BTN_XX_RISING;
                p_task_sensor_dta->tick = p_task_sensor_cfg->tick_max;
                p_task_sensor_dta->tick_hold = p_task_sensor_cfg->tick_hold_max;
            }
            break;

        case ST_BTN_XX_RISING:
            if (EV_BTN_XX_UP == p_task_sensor_dta->event) {
                if (DEL_BTN_XX_MIN == p_task_sensor_dta->tick) {
                    p_task_sensor_dta->state = ST_BTN_XX_UP;
                    if (p_task_sensor_cfg->signal_up != EV_SYS_NONE) {
                        put_event_task_menu(p_task_sensor_cfg->signal_up);
                    }
                } else {
                    p_task_sensor_dta->tick--;
                }
            }
            if (EV_BTN_XX_DOWN == p_task_sensor_dta->event) {
                if (DEL_BTN_XX_MIN == p_task_sensor_dta->tick) {
                    p_task_sensor_dta->state = ST_BTN_XX_DOWN;
                } else {
                    p_task_sensor_dta->tick--;
                }
            }
            break;

        default:
            p_task_sensor_dta->tick = DEL_BTN_XX_MIN;
            p_task_sensor_dta->tick_hold = DEL_BTN_XX_MIN;
            p_task_sensor_dta->state = ST_BTN_XX_UP;
            p_task_sensor_dta->event = EV_BTN_XX_UP;
            break;
        }
    }
}
