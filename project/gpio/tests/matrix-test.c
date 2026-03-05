#include "matrix.h"


void notmain(void) {
    matrix_init();
    while (1) {
        block_t block = read_block();
        if (block) {
            trace("Block pressed: %d\n", block);
        }
        delay_ms(100);
    }
}