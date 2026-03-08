#include "arcade.h"
#include "../world-state/world.h"
void notmain(void) {
    arcade_init();
    pos_t pos = {0, 0, 0};
    while (1) {
        arcade_read(&pos);
        trace("pos: %d, %d, %d\n", pos.x, pos.y, pos.z);
        delay_ms(100);
    }
}