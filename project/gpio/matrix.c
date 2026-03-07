#include "matrix.h"

void matrix_init(void) {
    for (int i = 16; i < 20; i++) {
        gpio_set_output(i);
    }
    for (int i = 20; i < 24; i++) {
        gpio_set_input(i);
        gpio_set_pullup(i);
    }
}

block_t read_block(void) {
    for (int col = 0; col < 4; col++) {
        for (int x = 0; x < 4; x++) {
            gpio_write(COL_BASE + x, x != col);
        }

        delay_us(5);

        for (int row = 0; row < 4; row++) {
            if (!gpio_read(ROW_BASE + row))
                return 16 - ((col * 4) + row);
        }
    }
    return 0;
}
