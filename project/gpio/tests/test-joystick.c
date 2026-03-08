#include "joystick.h"
#include "../world-state/player.h"
void notmain(void) {
    p_rot_t rot = {0, 0};
    uart_init();
    joystick_init();
    while (1) {
        read_joystick(&rot);
        delay_ms(100);
    }
}