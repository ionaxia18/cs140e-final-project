#include "arcade.h"

void arcade_init(void) {
    dev_barrier();
    for (int i =20; i <= 26; i+=2) {
        gpio_set_input(i);
        gpio_set_pullup(i);
    }
    dev_barrier();
}

void arcade_read(pos_t* pos) {
    dev_barrier();
    // trace("R=%d L=%d U=%d D=%d\n",gpio_read(PIN_RIGHT),!gpio_read(PIN_LEFT),gpio_read(PIN_UP),gpio_read(PIN_DOWN));
    if (!gpio_read(PIN_RIGHT)) pos->x += 1;
    if (gpio_read(PIN_LEFT))  pos->x += -1;
    if (!gpio_read(PIN_UP))    pos->y += 1;
    if (!gpio_read(PIN_DOWN))  pos->y += -1;
    dev_barrier();
}