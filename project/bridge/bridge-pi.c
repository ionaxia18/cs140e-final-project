#include "bridge-pi.h"
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define SERIAL_PORT "/dev/cu.SLAB_USBtoUART"
#define BAUDRATE B115200

struct termios old_t;

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_t);
    printf("raw keyboard mode disabled\n");
}
void enable_raw_keyboard_mode() {
    tcgetattr(STDIN_FILENO, &old_t);
    atexit(disable_raw_mode);

    struct termios new_t = old_t;

    new_t.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_t);
    printf("raw keyboard mode enabled\n");
}

int main() {
    int fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        return 1;
    }
    printf("fd opened\n");
    
    struct termios tty;
    tcgetattr(fd, &tty);
    cfsetspeed(&tty, BAUDRATE);
    tty.c_cflag |= (CREAD|CLOCAL);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tcsetattr(fd, TCSANOW, &tty);
    enable_raw_keyboard_mode();
    printf("press WASD to move, P to place block, R to remove block and Q to quit\n");

    while (1) {
        char c;
        // if no error
        if (read(STDIN_FILENO, &c, 1) != 1) continue;
        if (c == 'w' || c == 'a' || c == 's' || c == 'd' || c == 'p' || c == 'r') {
            printf("sending char %c\n", c);
            write(fd, &c, 1);
        }
        else if (c == 'q') {
            write(fd, &c, 1);
            printf("quitting the program\n");
            break;
        }
    }
    close(fd);
    return 0;
}
