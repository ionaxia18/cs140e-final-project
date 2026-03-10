#include "matrix.h"

void matrix_init(void) {
    for (int i = 0; i < 4; i++) {
        gpio_set_output(col_pins[i]);
    }

    for (int i = 0; i < 4; i++) {
        gpio_set_input(row_pins[i]);
        gpio_set_pullup(row_pins[i]);
    }
}

block_t read_block(void) {
    for (int col = 0; col < 4; col++) {
        // drive exactly one column low, others high
        for (int x = 0; x < 4; x++) {
            gpio_write(col_pins[x], x != col);
        }

        delay_us(5);

        for (int row = 0; row < 4; row++) {
            if (!gpio_read(row_pins[row])) {
                return (row * 4) + col + 1;
            }
        }
    }
    return 0;
}