#include "joystick.h"

void notmain(void) {
    uart_init();
    joystick_init();
    while (1) {
        read_joystick();
        delay_ms(100);
    }
}