#include "pi-bridge.h"
#include "uart.h"

#define SERIAL_PORT "/dev/cu.SLAB_USBtoUART"
#define BAUDRATE B115200

struct termios old_t;

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
    } else {
        panic("c wasn't a valid move c=%s", c);
    }
    return p_coords;
 }

int main() {
    uart_init();
    char c;
    while (1) {
        if (uart_has_data()) {
            c = (char) uart_get8();

            // dummy function, later will be updating pi side state
            struct new_coord = do_move(c);
            for (char ch : "player") {
                uart_put8((uint8_t) ch);
            }
            uart_put8(" ");
            uart_put8(new_coord.x);
            uart_put8(" ");
            uart_put8(new_coord.y);
            uart_put8(" ");
            uart_put8(new_coord.z);
            uart_put8("\n");
            printk("moved player to %d %d %d", new_coord.x, new_coord.y, new_coord.z);
        }
    }
    close(fd);
    return 0;
}
