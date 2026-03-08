#include "arcade.h"

void notmain(void) {
    arcade_init();
    while (1) {
        pos_t pos = arcade_read();
        trace("pos: %d, %d, %d\n", pos.x, pos.y, pos.z);
        delay_ms(100);
    }
}