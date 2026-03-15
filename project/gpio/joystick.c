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

int read_camera_joystick(p_rot_t* rot) {
    dev_barrier();
    int cx = read_channel(spi, 7) - 512;
    int cy = -1 * (read_channel(spi, 6) - 512);
    if (cx > -60 && cx < 60) cx = 0;
    if (cy > -60 && cy < 60) cy = 0;
    int delete = read_channel(spi, 5);
    // need to normalize so it goes back to 0 when it reaches a certain value
    cx /= -100;
    cy /= -125;
    rotation_increment(rot, cx, cy);
    // trace("move displacement= %d, %d, place=%d\n", cx, cy, delete);

    dev_barrier();
    return (delete < 512);
}

int read_move_joystick(pos_t* pos) {
    int mx = read_channel(spi, 2) - 512;
    int mz = -1 * (read_channel(spi, 1) - 512);
    int destroy = read_channel(spi, 0);

    if (mx > -80 && mx < 80) mx = 0;
    if (mz > -80 && mz < 80) mz = 0;
    // trace("move displacement= %d, %d, %d, place=%d\n", pos->x, pos->y, pos->z, destroy);
    // call function to move player
    if (mx > 0) {
        pos->x = 0.5;
    } else if (mx < 0) {
        pos->x = -0.5;
    }

    if (mz > 0) {
        pos->z = -0.5;
    } else if (mz < 0) {
        pos->z = 0.5;
    }
    // trace("move displacement= %d, %d, %d, place=%d\n", pos->x != 0, pos->y != 0, pos->z != 0, destroy);
    dev_barrier();
    return destroy < 512;
    // rotation_increment(rot, mx, my);
}