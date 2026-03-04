#include "bridge-pi.h"
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <ApplicationServices/ApplicationServices.h>
#include "../constants.h"

#define SEND_INTERVAL 0.03
#define BAUDRATE B115200

int fd;

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

CGEventRef trackpad_event_handler(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
    static int accum_dx = 0;
    static int accum_dy = 0;
    static struct timespec last_send = {0,0};

    if (type != kCGEventScrollWheel) {
        return event;
    }
    int dx = (int)CGEventGetDoubleValueField(event, kCGScrollWheelEventDeltaAxis1);
    int dy = (int)CGEventGetDoubleValueField(event, kCGScrollWheelEventDeltaAxis2);  
    accum_dx += dx;
    accum_dy += dy;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    if (now.tv_sec - last_send.tv_sec + (now.tv_nsec - last_send.tv_nsec) / 1000000000.0 > SEND_INTERVAL) {
        char buffer[32];
        int len = snprintf(buffer, sizeof(buffer), "m %d %d\n", accum_dx, accum_dy);
        printf("sending mouse move %d %d\n", accum_dx, accum_dy);
        write(fd, buffer, len);\
        accum_dx = 0;
        accum_dy = 0;
        last_send = now;
    }
    return event;
}

void *trackpad_loop_thread(void *arg) {
    CGEventMask want_event = CGEventMaskBit(kCGEventScrollWheel);
    CFMachPortRef tap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, 0, want_event, trackpad_event_handler, NULL);

    if (tap == NULL) {
        printf("failed to create event tap\n");
        return NULL;
    }
    CFRunLoopSourceRef source = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, tap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopDefaultMode);
    printf("event tap created and added to run loop\n");
    CFRunLoopRun();
    printf("run loop exited\n");
    return NULL;
}

int main() {
    printf("starting bridge-pi\n");
    fd = open(PI_PORT, O_RDWR | O_NOCTTY);
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
    printf("press WASD to move, P to place block, R to remove block, and Q to quit. Use trackpad scroll to move the camera.\n");

    pthread_t trackpad_thread;
    pthread_create(&trackpad_thread, NULL, trackpad_loop_thread, NULL);

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
            pthread_cancel(trackpad_thread);
            break;
        }
    }
    close(fd);
    return 0;
}
