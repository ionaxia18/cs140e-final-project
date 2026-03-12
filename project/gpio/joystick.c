#include "joystick.h"
#include "../pi-side/uart-helpers.h"
#include "../world-state/player.h"
static spi_t spi;

void joystick_init(void) {
    dev_barrier();
    spi = spi_n_init(SPI_CE0, 256);
    dev_barrier();
}

int read_channel(spi_t s, uint8_t channel) {
    dev_barrier();
    uint8_t tx[3] = {0x01, 0x80 | (channel << 4), 0};
    uint8_t rx[3] = {0, 0, 0};
    spi_n_transfer(s, rx, tx, 3);
    return ((rx[1] & 0x03) << 8) | rx[2];
    dev_barrier();
}

int read_joystick(p_rot_t* rot) {
    dev_barrier();
    int x  = read_channel(spi, 2) - 512;
    int y  = -1 * (read_channel(spi, 1) - 512);
    if (x > -60 && x < 60) x = 0;
    if (y > -60 && y < 60) y = 0;
    int sw = read_channel(spi, 0);
    // need to normalize so it goes back to 0 when it reaches a certain value
    x /= -100;
    y /= -125;
    rotation_increment(rot, x, y);
    dev_barrier();
    // trace("x: %d, y: %d, button: %d\n", x, y, sw < 512);
    return (sw < 512);
    // if (sw < 512) {
    //     gpio_set_on(LED_PIN);
    // } else {
    //     gpio_set_off(LED_PIN);
    // }
}
