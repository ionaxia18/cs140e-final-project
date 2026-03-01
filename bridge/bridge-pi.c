#include "uart.h"
#include "bridge-pi.h"

void bridge_init(void) {
    uart_init();
}

void send_string(const char *s) {
    while (*s) {
        uart_put8(*s);
        s++;
    }
}

void read_line(char * buf, int max_size) {
    int i = 0;
    while (i < max_size) {
        char c = uart_get8();
        if (c == '\n') {
            break;
        }
        buf[i] = c;
        i += 1;
    }
    buf[i] = '\0';
    return i;
}

