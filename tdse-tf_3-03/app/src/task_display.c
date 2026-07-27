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
    char vram_waiting[2][16]; // Waiting buffer (written by the application via LCD_show)
    char vram_active[2][16];  // Active buffer (currently being rendered to the LCD)
    bool dirty;               // Synchronization flag (true when new data is available)
} task_display_dta_t;

static task_display_dta_t display_task_dta;

// Private helper function to format and optionally center a single row
static void write_line_to_buffer(char *buffer_row, const char *line, bool center) {
    // 1. Clear the entire row with blank spaces
    memset(buffer_row, ' ', 16);

    if (line == NULL) {
        return;
    }

    size_t len = strlen(line);
    if (len > 16) {
        len = 16; // Safety truncation to prevent buffer overflow
    }

    uint8_t offset = 0;
    if (center && len < 16) {
        // Calculate the left margin needed to center the text
        offset = (uint8_t)((16 - len) / 2);
    }

    // Copy the string into the buffer at the calculated offset position
    memcpy(buffer_row + offset, line, len);
}

void task_display_init(void *parameters) {
    displayInit(DISPLAY_CONNECTION_GPIO_4BITS);
    display_task_dta.state = ST_DISPLAY_IDLE;
    display_task_dta.row = 0;
    display_task_dta.col = 0;
    display_task_dta.dirty = false;

    // Initialize both display buffers with empty spaces
    memset(display_task_dta.vram_waiting, ' ', sizeof(display_task_dta.vram_waiting));
    memset(display_task_dta.vram_active, ' ', sizeof(display_task_dta.vram_active));

    // Display an initial centered test message
    LCD_show("SMARTCETA", "Iniciando...", CENTER);
}

void LCD_show_ext(const char *line1, const char *line2, bool center) {
    // 1. Create a local temporary "shadow buffer"
    // (Assuming 16 characters wide + null terminator '\0')
    char temp_buffer[2][17];

    // 2. Process all text OUTSIDE the critical section.
    // During this string formatting, interrupts remain enabled
    // (ensuring time-sensitive tasks like the DHT22 are unaffected).
    write_line_to_buffer(temp_buffer[0], line1, center);
    write_line_to_buffer(temp_buffer[1], line2, center);

    // 3. MINIMIZED CRITICAL SECTION
    // Disable interrupts only for the exact duration needed to copy the already-processed memory.
    __asm("CPSID i");

    memcpy(display_task_dta.vram_waiting[0], temp_buffer[0], 16);
    memcpy(display_task_dta.vram_waiting[1], temp_buffer[1], 16);
    display_task_dta.dirty = true;

    __asm("CPSIE i");
}

/* State machine: Executed periodically within the cooperative scheduler's cycle */
void task_display_update(void *parameters) {
    switch (display_task_dta.state) {

        case ST_DISPLAY_IDLE:
            // Check if the application has provided new display data
            if (display_task_dta.dirty) {
                __asm("CPSID i"); // Enter critical section
                memcpy(display_task_dta.vram_active, display_task_dta.vram_waiting, sizeof(display_task_dta.vram_active));
                display_task_dta.dirty = false;
                __asm("CPSIE i"); // Exit critical section

                // Reset cursor position tracking and trigger the rendering process
                display_task_dta.row = 0;
                display_task_dta.col = 0;
                display_task_dta.state = ST_DISPLAY_SET_ROW;
            }
            break;

        case ST_DISPLAY_SET_ROW:
            // Position the physical cursor at the beginning of the current row (column 0)
            displayCharPositionWrite(0, display_task_dta.row);
            display_task_dta.state = ST_DISPLAY_WRITE_CHAR;
            break;

        case ST_DISPLAY_WRITE_CHAR: {
            // Send a SINGLE character per tick to avoid blocking the scheduler
            char temp_str[2];
            temp_str[0] = display_task_dta.vram_active[display_task_dta.row][display_task_dta.col];
            temp_str[1] = '\0';

            displayStringWrite(temp_str);

            display_task_dta.col++;
            if (display_task_dta.col >= 16) {
                // End of row reached, move to the next one
                display_task_dta.col = 0;
                display_task_dta.row++;

                if (display_task_dta.row >= 2) {
                    // Entire screen updated, return to idle state
                    display_task_dta.state = ST_DISPLAY_IDLE;
                } else {
                    // Move to the next row (requires physical cursor repositioning)
                    display_task_dta.state = ST_DISPLAY_SET_ROW;
                }
            }
            // NOTE: If col < 16, the LCD hardware auto-increments the cursor internally,
            // so the next scheduler tick will seamlessly send the following character.
            break;
        }

        default:
            display_task_dta.state = ST_DISPLAY_IDLE;
            break;
    }
}
