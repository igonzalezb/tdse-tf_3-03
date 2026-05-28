#include "main.h"
#include "app.h"
#include "board.h"
#include "task_dht22.h"

extern TIM_HandleTypeDef htim2;

typedef enum {
    TASK_DHT22_ST_IDLE = 0,
    TASK_DHT22_ST_START_LOW,
    TASK_DHT22_ST_WAIT_CAPTURE
} task_dht22_state_t;

typedef struct {
    task_dht22_state_t state;
    uint32_t state_timestamp_ms;
    uint32_t last_sample_ms;
} task_dht22_data_t;

typedef struct {
    volatile bool active;
    volatile uint32_t edge_count;
    volatile uint32_t last_edge_us;
    volatile uint32_t dt_us[DHT22_MAX_EDGES];
    volatile uint8_t level_after_edge[DHT22_MAX_EDGES];
} dht22_capture_t;

static task_dht22_data_t task_dht22_data;
static dht22_capture_t dht22_capture;

static bool dht22_time_elapsed(uint32_t now, uint32_t since, uint32_t period)
{
    return ((uint32_t)(now - since) >= period);
}

static void dht22_set_pin_output_low(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    HAL_GPIO_WritePin(DHT22_GPIO_PORT, DHT22_GPIO_PIN, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = DHT22_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT22_GPIO_PORT, &GPIO_InitStruct);
}

static void dht22_set_pin_input_plain(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT22_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DHT22_GPIO_PORT, &GPIO_InitStruct);
}

static void dht22_set_pin_input_exti(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT22_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DHT22_GPIO_PORT, &GPIO_InitStruct);
}

static void dht22_capture_start(void)
{
    dht22_capture.active = false;
    dht22_capture.edge_count = 0u;
    dht22_capture.last_edge_us = __HAL_TIM_GET_COUNTER(&htim2);

    dht22_set_pin_input_exti();

    dht22_capture.active = true;
}

static void dht22_capture_stop(void)
{
    dht22_capture.active = false;
    dht22_set_pin_input_plain();
}

static bool dht22_decode(uint16_t *humidity_x10, int16_t *temperature_x10, uint8_t *error_code)
{
    uint8_t data[5] = {0u, 0u, 0u, 0u, 0u};
    uint32_t i;
    uint32_t falling_count = 0u;
    uint32_t bit_index = 0u;
    uint16_t raw_humidity;
    uint16_t raw_temperature;
    uint8_t checksum;

    for (i = 0u; i < dht22_capture.edge_count; i++) {

        if (dht22_capture.level_after_edge[i] == GPIO_PIN_RESET) {
            falling_count++;

            /* Skip:
             * 1) initial falling edge after host release
             * 2) sensor response high (~80 us)
             */
            if (falling_count <= 2u) {
                continue;
            }

            if (bit_index >= 40u) {
                break;
            }

            data[bit_index / 8u] <<= 1u;

            if (dht22_capture.dt_us[i] > DHT22_HIGH_THRESHOLD_US) {
                data[bit_index / 8u] |= 1u;
            }

            bit_index++;
        }
    }

    if (bit_index != 40u) {
        *error_code = DHT22_ERROR_FRAME;
        return false;
    }

    checksum = (uint8_t)(data[0] + data[1] + data[2] + data[3]);
    if (checksum != data[4]) {
        *error_code = DHT22_ERROR_CHECKSUM;
        return false;
    }

    raw_humidity = (uint16_t)(((uint16_t)data[0] << 8u) | data[1]);
    raw_temperature = (uint16_t)(((uint16_t)data[2] << 8u) | data[3]);

    *humidity_x10 = raw_humidity;

    if ((raw_temperature & 0x8000u) != 0u) {
        *temperature_x10 = -(int16_t)(raw_temperature & 0x7FFFu);
    } else {
        *temperature_x10 = (int16_t)raw_temperature;
    }

    *error_code = DHT22_ERROR_NONE;
    return true;
}

void task_dht22_init(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;

    task_dht22_data.state = TASK_DHT22_ST_IDLE;
    task_dht22_data.state_timestamp_ms = HAL_GetTick();
    task_dht22_data.last_sample_ms = HAL_GetTick();

    dht22_capture.active = false;
    dht22_capture.edge_count = 0u;
    dht22_capture.last_edge_us = 0u;

    dht22_set_pin_input_plain();

    shared_data->dht22_humidity_x10 = 0u;
    shared_data->dht22_temperature_x10 = 0;
    shared_data->dht22_valid = false;
    shared_data->dht22_changed = false;
    shared_data->dht22_error = DHT22_ERROR_NONE;
}

void task_dht22_update(void *parameters)
{
    shared_data_type *shared_data = (shared_data_type *) parameters;
    uint32_t now_ms;
    uint16_t humidity_x10;
    int16_t temperature_x10;
    uint8_t error_code;
    bool changed;

    shared_data->dht22_changed = false;
    now_ms = HAL_GetTick();

    switch (task_dht22_data.state)
    {
    case TASK_DHT22_ST_IDLE:

        if (!dht22_time_elapsed(now_ms, task_dht22_data.last_sample_ms, DHT22_SAMPLE_PERIOD_MS)) {
            break;
        }

        dht22_set_pin_output_low();
        task_dht22_data.state_timestamp_ms = now_ms;
        task_dht22_data.state = TASK_DHT22_ST_START_LOW;
        break;

    case TASK_DHT22_ST_START_LOW:

        if (!dht22_time_elapsed(now_ms, task_dht22_data.state_timestamp_ms, DHT22_START_LOW_MS)) {
            break;
        }

        dht22_capture_start();
        task_dht22_data.state_timestamp_ms = now_ms;
        task_dht22_data.state = TASK_DHT22_ST_WAIT_CAPTURE;
        break;

    case TASK_DHT22_ST_WAIT_CAPTURE:

        if (!dht22_time_elapsed(now_ms, task_dht22_data.state_timestamp_ms, DHT22_CAPTURE_WINDOW_MS)) {
            break;
        }

        dht22_capture_stop();

        if (dht22_decode(&humidity_x10, &temperature_x10, &error_code) == true) {

            changed = (!shared_data->dht22_valid) ||
                      (shared_data->dht22_humidity_x10 != humidity_x10) ||
                      (shared_data->dht22_temperature_x10 != temperature_x10);

            shared_data->dht22_humidity_x10 = humidity_x10;
            shared_data->dht22_temperature_x10 = temperature_x10;
            shared_data->dht22_valid = true;
            shared_data->dht22_error = DHT22_ERROR_NONE;
            shared_data->dht22_changed = changed;
        }
        else {
            changed = shared_data->dht22_valid;

            shared_data->dht22_valid = false;
            shared_data->dht22_error = error_code;
            shared_data->dht22_changed = changed;
        }

        task_dht22_data.last_sample_ms = now_ms;
        task_dht22_data.state = TASK_DHT22_ST_IDLE;
        break;

    default:
        dht22_capture_stop();
        task_dht22_data.state = TASK_DHT22_ST_IDLE;
        task_dht22_data.last_sample_ms = now_ms;
        break;
    }
}

void task_dht22_exti_callback(uint16_t GPIO_Pin)
{
    uint32_t now_us;
    uint32_t dt_us;
    GPIO_PinState level;

    if (GPIO_Pin != DHT22_GPIO_PIN) {
        return;
    }

    if (dht22_capture.active == false) {
        return;
    }

    now_us = __HAL_TIM_GET_COUNTER(&htim2);
    dt_us = now_us - dht22_capture.last_edge_us;
    dht22_capture.last_edge_us = now_us;

    level = HAL_GPIO_ReadPin(DHT22_GPIO_PORT, DHT22_GPIO_PIN);

    if (dht22_capture.edge_count < DHT22_MAX_EDGES) {
        dht22_capture.dt_us[dht22_capture.edge_count] = dt_us;
        dht22_capture.level_after_edge[dht22_capture.edge_count] = (uint8_t)level;
        dht22_capture.edge_count++;
    }
}
