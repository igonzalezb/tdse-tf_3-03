#include "task_actuator_interface.h"

#define EVENT_UNDEFINED (0xFFFFFFFF)
#define MAX_EVENTS      (8) // Tamaño de cada cola individual

typedef struct {
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    uint32_t queue[MAX_EVENTS];
} queue_t;

// Creamos un array de colas: una para cada actuador
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
    // Protección ante desborde de ID por seguridad
    if (id >= ID_ACT_QTY) return;

    queue_t *p_q = &actuator_queues[id];

    // Si la cola está llena, podrías descartar o manejar el error. Aquí sobreescribe/avanza.
    p_q->count++;
    p_q->queue[p_q->head++] = event;

    if (MAX_EVENTS == p_q->head) {
        p_q->head = 0;
    }
}

uint32_t get_event_task_actuator(task_actuator_id_t id) {
    if (id >= ID_ACT_QTY) return EVENT_UNDEFINED;

    queue_t *p_q = &actuator_queues[id];
    uint32_t event = EVENT_UNDEFINED;

    if (p_q->count > 0) {
        p_q->count--;
        event = p_q->queue[p_q->tail++];

        if (MAX_EVENTS == p_q->tail) {
            p_q->tail = 0;
        }
    }
    return event;
}

bool any_event_task_actuator(task_actuator_id_t id) {
    if (id >= ID_ACT_QTY) return false;
    return (actuator_queues[id].count > 0);
}
