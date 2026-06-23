/* task_display.c */
#include "task_display.h"
#include "display.h"   // Para usar tus funciones físicas existentes
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

void task_display_init(void *parameters) {
	displayInit(DISPLAY_CONNECTION_GPIO_4BITS);
    display_task_dta.state = ST_DISPLAY_IDLE;
    display_task_dta.row = 0;
    display_task_dta.col = 0;
    display_task_dta.dirty = false;

    // Inicializar buffers con espacios vacíos
    memset(display_task_dta.vram_waiting, ' ', sizeof(display_task_dta.vram_waiting));
    memset(display_task_dta.vram_active, ' ', sizeof(display_task_dta.vram_active));
    LCD_show("    SMARTCETA   ", "   Iniciando...  ");
}


void LCD_show(const char *line1, const char *line2) {
    // Limpiamos el buffer de espera con espacios
    memset(display_task_dta.vram_waiting, ' ', sizeof(display_task_dta.vram_waiting));

    // Copiamos la línea 1 si no es nula
    if (line1 != NULL) {
        size_t len1 = strlen(line1);
        if (len1 > 16) len1 = 16;
        memcpy(display_task_dta.vram_waiting[0], line1, len1);
    }

    // Copiamos la línea 2 si no es nula
    if (line2 != NULL) {
        size_t len2 = strlen(line2);
        if (len2 > 16) len2 = 16;
        memcpy(display_task_dta.vram_waiting[1], line2, len2);
    }

    // Avisamos a la máquina de estados que hay contenido nuevo listo
    display_task_dta.dirty = true;
}

/* Máquina de estados: se ejecuta dentro del ciclo del planificador cooperativo */
void task_display_update(void *parameters) {
    switch (display_task_dta.state) {

        case ST_DISPLAY_IDLE:
            // Si la aplicación modificó el texto, pasamos el buffer al render activo de forma atómica
            if (display_task_dta.dirty) {
                __asm("CPSID i"); // Deshabilitar interrupciones momentáneamente (protección de recurso compartido)
                memcpy(display_task_dta.vram_active, display_task_dta.vram_waiting, sizeof(display_task_dta.vram_active));
                display_task_dta.dirty = false;
                __asm("CPSIE i"); // Reestablecer interrupciones

                display_task_dta.row = 0;
                display_task_dta.col = 0;
                display_task_dta.state = ST_DISPLAY_SET_ROW;
            }
            break;

        case ST_DISPLAY_SET_ROW:
            // Posiciona el cursor físico al inicio de la fila actual (columna 0) utilizando tu función existente
            displayCharPositionWrite(0, display_task_dta.row);
            display_task_dta.state = ST_DISPLAY_WRITE_CHAR;
            break;

        case ST_DISPLAY_WRITE_CHAR: {
            // Mandamos un ÚNICO carácter por ciclo.
            // Para no alterar tu función displayStringWrite, armamos un string temporal de longitud 1.
            char temp_str[2];
            temp_str[0] = display_task_dta.vram_active[display_task_dta.row][display_task_dta.col];
            temp_str[1] = '\0';

            // Llama a tu función original. Conserva TODOS sus tiempos de delay internos por comando/bus,
            // pero al procesar un solo carácter el bloqueo en este ciclo es imperceptible.
            displayStringWrite(temp_str);

            display_task_dta.col++;
            if (display_task_dta.col >= 16) {
                display_task_dta.col = 0;
                display_task_dta.row++;

                if (display_task_dta.row >= 2) {
                    // Terminamos de actualizar toda la pantalla física
                    display_task_dta.state = ST_DISPLAY_IDLE;
                } else {
                    // Cambiamos de fila: exige reposicionar el cursor mediante ST_DISPLAY_SET_ROW
                    display_task_dta.state = ST_DISPLAY_SET_ROW;
                }
            }
            // NOTA: Si col < 16, el hardware del LCD avanza el cursor solo,
            // por lo que el próximo tick enviará el siguiente carácter consecutivamente.
            break;
        }

        default:
            display_task_dta.state = ST_DISPLAY_IDLE;
            break;
    }
}
