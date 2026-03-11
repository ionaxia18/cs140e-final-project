#include "arcade.h"

void arcade_init(void) {
    // dev_barrier();
    for (int i = 20; i <= 26; i+=2) {
        gpio_set_input(i);
        gpio_set_pullup(i);
    }
    dev_barrier();
}
// right = 24, left = 26, up = 20, down = 22

pos_t arcade_read() {
    // dev_barrier();
    // trace("R=%d L=%d U=%d D=%d\n",gpio_read(PIN_RIGHT),gpio_read(PIN_LEFT),gpio_read(PIN_UP),gpio_read(PIN_DOWN));
    pos_t displacement = {0, 0, 0};
    if (!gpio_read(PIN_LEFT)) {
        displacement.x += 1;
    }
    if (!gpio_read(PIN_RIGHT)) {
        displacement.x += -1;
    }

    if (!gpio_read(PIN_UP)) {
        displacement.z += 1;
    }
    if (!gpio_read(PIN_DOWN)) {
        displacement.z += -1;
    }
    dev_barrier();
    return displacement;
}