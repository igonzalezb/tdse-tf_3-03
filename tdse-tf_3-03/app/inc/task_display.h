/* task_display.h */
#ifndef INC_TASK_DISPLAY_H_
#define INC_TASK_DISPLAY_H_

#include <stdint.h>
#include <stdbool.h>

#ifndef CENTER
#define CENTER true
#endif

// Inicialización de la tarea y buffers
void task_display_init(void *parameters);

// Tarea periódica no bloqueante para el planificador
void task_display_update(void *parameters);

// Función base extendida con parámetro de centrado
void LCD_show_ext(const char *line1, const char *line2, bool center);

// Detecta si envías 2 o 3 argumentos y llama a la macro correspondiente
#define GET_LCD_SHOW_MACRO(_1, _2, _3, NAME, ...) NAME
#define LCD_show(...) GET_LCD_SHOW_MACRO(__VA_ARGS__, LCD_show_3, LCD_show_2)(__VA_ARGS__)

// Mapeo de llamadas:
#define LCD_show_2(line1, line2)         LCD_show_ext((line1), (line2), false)
#define LCD_show_3(line1, line2, center) LCD_show_ext((line1), (line2), (center))

#endif /* INC_TASK_DISPLAY_H_ */
