#include "pi_bridge.h"
#include "../../libpi/rpi.h"

#define SERIAL_PORT "/dev/cu.usbserial-110"
#define BAUDRATE B115200

struct coords p_coords = {235, 65, -1};
char * block = "STONE";

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

// takes in a character move, will move the player on pi side and return new coordinates
void do_move(char c) {
    if (c == 'w') {
        p_coords.x += 1;
    } else if (c == 'a') {
        p_coords.y -= 1;
    } else if (c == 's') {
        p_coords.x += 1;
    } else if (c == 'd') {
        p_coords.y -= 1;
    }

    const char *str = "PLAYER";
    for (size_t i = 0; str[i] != '\0'; i++) {
        uart_put8((uint8_t) str[i]);
    }
    uart_put8(' ');
    uart_put_int(p_coords.x);
    uart_put_int(p_coords.y);
    uart_put_int(p_coords.z);
    uart_put8('\n');
 }

void change_block(char c) {
    char * cur_block = "";
    if (c == 'p') {
        cur_block = block;
    }
    else if (c == 'r') {
        cur_block = "AIR";
    }
    const char *str = "BLOCK";
    for (size_t i = 0; str[i] != '\0'; i++) {
        uart_put8((uint8_t) str[i]);
    }
    uart_put8(' ');
    uart_put_int(p_coords.x);
    uart_put_int(p_coords.y);
    uart_put_int(p_coords.z);
    for (size_t i = 0; cur_block[i] != '\0'; i++) {
        uart_put8((uint8_t) cur_block[i]);
    }
    uart_put8('\n');
}



void notmain() {
    uart_init();
    while (1) {
        if (uart_has_data()) {
            char c = (char) uart_get8();
            if (c == 'w' || c == 'a' || c == 's' || c == 'd') {
                do_move(c);
            }
            else if (c == 'p' || c == 'r') {
                change_block(c);
            } 
            else if (c == 'q') {
                uart_flush_tx();
                return;
            }
        }
    }
}
