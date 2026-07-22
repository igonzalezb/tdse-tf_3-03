/* task_display.c */
#include "task_display.h"
#include "display.h"
#include <string.h>

typedef enum {
    ST_DISPLAY_IDLE,
    ST_DISPLAY_SET_ROW,
    ST_DISPLAY_WRITE_CHAR
} task_display_state_t;

typedef struct {
    task_display_state_t state;
    uint8_t row;
    uint8_t col;
    char vram_waiting[2][16]; // Buffer donde escribe la app (LCD_show)
    char vram_active[2][16];  // Buffer que se está imprimiendo actualmente
    bool dirty;               // Flag de sincronización
} task_display_dta_t;

static task_display_dta_t display_task_dta;

// Función auxiliar privada para formatear y centrar una fila
static void write_line_to_buffer(char *buffer_row, const char *line, bool center) {
    // 1. Limpiamos la fila completa con espacios
    memset(buffer_row, ' ', 16);

    if (line == NULL) {
        return;
    }

    size_t len = strlen(line);
    if (len > 16) {
        len = 16; // Recorte de seguridad si excede los 16 caracteres
    }

    uint8_t offset = 0;
    if (center && len < 16) {
        // Cálculo matemático del margen para centrar el texto
        offset = (uint8_t)((16 - len) / 2);
    }

    // Copiamos la cadena en la posición desplazada correspondiente
    memcpy(buffer_row + offset, line, len);
}

void task_display_init(void *parameters) {
    displayInit(DISPLAY_CONNECTION_GPIO_4BITS);
    display_task_dta.state = ST_DISPLAY_IDLE;
    display_task_dta.row = 0;
    display_task_dta.col = 0;
    display_task_dta.dirty = false;

    // Inicializar buffers con espacios vacíos
    memset(display_task_dta.vram_waiting, ' ', sizeof(display_task_dta.vram_waiting));
    memset(display_task_dta.vram_active, ' ', sizeof(display_task_dta.vram_active));

    // Mensaje inicial centrado de prueba
    LCD_show("SMARTCETA", "Iniciando...", CENTER);
}

void LCD_show_ext(const char *line1, const char *line2, bool center) {
    __asm("CPSID i"); // Protección de Sección Crítica (Deshabilita interrupciones)

    // Formateamos ambas líneas en el buffer de espera
    write_line_to_buffer(display_task_dta.vram_waiting[0], line1, center);
    write_line_to_buffer(display_task_dta.vram_waiting[1], line2, center);

    // Avisamos a la máquina de estados que hay contenido nuevo listo
    display_task_dta.dirty = true;

    __asm("CPSIE i"); // Reestablece interrupciones
}

/* Máquina de estados: se ejecuta dentro del ciclo del planificador cooperativo */
void task_display_update(void *parameters) {
    switch (display_task_dta.state) {

        case ST_DISPLAY_IDLE:
            if (display_task_dta.dirty) {
                __asm("CPSID i");
                memcpy(display_task_dta.vram_active, display_task_dta.vram_waiting, sizeof(display_task_dta.vram_active));
                display_task_dta.dirty = false;
                __asm("CPSIE i");

                display_task_dta.row = 0;
                display_task_dta.col = 0;
                display_task_dta.state = ST_DISPLAY_SET_ROW;
            }
            break;

        case ST_DISPLAY_SET_ROW:
            displayCharPositionWrite(0, display_task_dta.row);
            display_task_dta.state = ST_DISPLAY_WRITE_CHAR;
            break;

        case ST_DISPLAY_WRITE_CHAR: {
            char temp_str[2];
            temp_str[0] = display_task_dta.vram_active[display_task_dta.row][display_task_dta.col];
            temp_str[1] = '\0';

            displayStringWrite(temp_str);

            display_task_dta.col++;
            if (display_task_dta.col >= 16) {
                display_task_dta.col = 0;
                display_task_dta.row++;

                if (display_task_dta.row >= 2) {
                    display_task_dta.state = ST_DISPLAY_IDLE;
                } else {
                    display_task_dta.state = ST_DISPLAY_SET_ROW;
                }
            }
            break;
        }

        default:
            display_task_dta.state = ST_DISPLAY_IDLE;
            break;
    }
}
