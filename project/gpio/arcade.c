#include "arcade.h"

void arcade_init(void) {
    for (int i =20; i <= 26; i+=2) {
        gpio_set_input(i);
        gpio_set_pullup(i);
    }
}

pos_t arcade_read(void) {
    pos_t pos = {0, 0, 0};
    if (!gpio_read(PIN_RIGHT)) pos.x = 5;
    if (!gpio_read(PIN_LEFT))  pos.x = -5;
    if (!gpio_read(PIN_UP))    pos.y = 5;
    if (!gpio_read(PIN_DOWN))  pos.y = -5;
    return pos;
}