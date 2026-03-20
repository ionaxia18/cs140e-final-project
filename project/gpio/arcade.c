// DEPRECATED, USED FOR MOVING PLAYER, BUT SWITCHED TO JOYSTICK. SEE joystick.c
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
    // left
    if (!gpio_read(PIN_LEFT)) {
        displacement.x += -0.5;
    }
    // right
    if (!gpio_read(PIN_RIGHT)) {
        displacement.x += 0.5;
    }

    // forward
    if (!gpio_read(PIN_UP)) {
        displacement.z += 0.5;
    }
    // backward
    if (!gpio_read(PIN_DOWN)) {
        displacement.z += -0.5;
    }
    dev_barrier();
    return displacement;
}