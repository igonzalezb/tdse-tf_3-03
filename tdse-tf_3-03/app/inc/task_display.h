/* task_display.h */
#ifndef INC_TASK_DISPLAY_H_
#define INC_TASK_DISPLAY_H_

#include <stdint.h>
#include <stdbool.h>

// Inicialización de la tarea y buffers
void task_display_init(void *parameters);

// Tarea periódica no bloqueante para el planificador
void task_display_update(void *parameters);

// Reemplazo directo y NO bloqueante para el LCD_show existente
void LCD_show(const char *line1, const char *line2);

#endif /* INC_TASK_DISPLAY_H_ */
