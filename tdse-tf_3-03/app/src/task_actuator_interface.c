#include "task_actuator_interface.h"

#define EVENT_UNDEFINED (255)
#define MAX_EVENTS      (16) // Tamaño de cada cola individual

typedef struct {
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    uint32_t queue[MAX_EVENTS];
} queue_t;

// Una cola para cada actuador
static queue_t actuator_queues[ID_ACT_QTY];

void init_queue_event_task_actuator(void) {
    for (int id = 0; id < ID_ACT_QTY; id++) {
        actuator_queues[id].head = 0;
        actuator_queues[id].tail = 0;
        actuator_queues[id].count = 0;
        for (int i = 0; i < MAX_EVENTS; i++) {
            actuator_queues[id].queue[i] = EVENT_UNDEFINED;
        }
    }
}

void put_event_task_actuator(task_actuator_id_t id, uint32_t event) {
    // Protección ante desborde de ID
    if (id >= ID_ACT_QTY) return;

    queue_t *queues = &actuator_queues[id];

    // Si la cola está llena, sobreescribe.
    queues->count++;
    queues->queue[queues->head++] = event;

    if (MAX_EVENTS == queues->head) {
        queues->head = 0;
    }
}

uint32_t get_event_task_actuator(task_actuator_id_t id) {
    if (id >= ID_ACT_QTY) return EVENT_UNDEFINED;

    queue_t *queues = &actuator_queues[id];
    uint32_t event = EVENT_UNDEFINED;

    if (queues->count > 0) {
        queues->count--;
        event = queues->queue[queues->tail++];

        if (MAX_EVENTS == queues->tail) {
            queues->tail = 0;
        }
    }
    return event;
}

bool any_event_task_actuator(task_actuator_id_t id) {
    if (id >= ID_ACT_QTY) return false;
    return (actuator_queues[id].count > 0);
}
