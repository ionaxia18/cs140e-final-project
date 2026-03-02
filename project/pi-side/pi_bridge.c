#include "pi_bridge.h"
#include "../../libpi/rpi.h"

#define SERIAL_PORT "/dev/cu.SLAB_USBtoUART"
#define BAUDRATE B115200

struct coords p_coords = {0, 0, 0};

// takes in a character move, will move the player on pi side and return new coordinates
coords do_move(char c) {
    if (c == 'w') {
        p_coords.x += 1;
    } else if (c == 'a') {
        p_coords.y -= 1;
    } else if (c == 's') {
        p_coords.x += 1;
    } else if (c == 'd') {
        p_coords.y -= 1;
    }
    return p_coords;
 }

 void uart_put_int(int val) {
    char buf[11];
    int i = 10;
    buf[i] = '\0';
    i -= 1;

    if (val < 0) {
        uart_put8('-');
        val = -val;
    }

    if (val == 0) buf[i] = '0';
    i -= 1;
    while (val > 0) {
        buf[i] = '0' + (val % 10);
        i -= 1;
        val /= 10;
    }
    i++;
    for (; buf[i] != '\0'; i++) {
        uart_put8(buf[i]);
    }
}

void notmain() {
    uart_init();
    while (1) {
        if (uart_has_data()) {
            char c = (char) uart_get8();

            // dummy function, later will be updating pi side state
            struct coords new_coord = do_move(c);
            
            const char *str = "player";
            for (size_t i = 0; str[i] != '\0'; i++) {
                uart_put8((uint8_t) str[i]);
            }
            uart_put8(' ');
            uart_put_int(new_coord.x);
            uart_put8(' ');
            uart_put_int(new_coord.y);
            uart_put8(' ');
            uart_put_int(new_coord.z);
            uart_put8('\n');
        }
    }
}
