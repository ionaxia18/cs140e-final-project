#include "joystick.h"

static spi_t spi;

void joystick_init(void) {
    spi = spi_n_init(SPI_CE0, 256);
    gpio_set_output(LED_PIN);
}

int read_channel(spi_t s, uint8_t channel) {
    uint8_t tx[3] = {0x01, 0x80 | (channel << 4), 0};
    uint8_t rx[3] = {0, 0, 0};
    spi_n_transfer(s, rx, tx, 3);
    return ((rx[1] & 0x03) << 8) | rx[2];
}

void read_joystick(void) {
    int x  = read_channel(spi, 1);
    int y  = read_channel(spi, 2);
    int sw = read_channel(spi, 0);
    if (sw < 512) {
        gpio_set_on(LED_PIN);
    } else {
        gpio_set_off(LED_PIN);
    }

    trace("x: %d, y: %d, button: %d\n", x, y, sw < 512);
}
