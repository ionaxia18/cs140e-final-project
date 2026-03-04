#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <termios.h>
#include <fcntl.h>

#define PLUGIN_IP "127.0.0.1"
#define PLUGIN_PORT 4711
#define PI_PORT "/dev/cu.usbserial-110"
#define BAUDRATE    B115200

// setup tcp function so that it can be used to talk to fruitjuice plugin
int connect_to_fruitjuice(const char * ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    printf("sock int created");
    if (sock < 0) {
        return -1;
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);
    // bridge needs to act as TCP client
    connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    return sock;
}

void put_block(int sock, int x, int y, int z, const char * block) {
    char buf[128];
    sprintf(buf, "world.setBlock(%d,%d,%d,%s)\n", x, y, z, block);
    send(sock, buf, strlen(buf), 0);
}

void move_player(int sock, int x, int y, int z) {
    char buf[128];
    sprintf(buf, "player.setTile(%d,%d,%d)\n", x, y, z);
    send(sock, buf, strlen(buf), 0);
}

void send_player_rotation(int sock, int dx, int dy) {
    char buf[128];
    sprintf(buf, "player.setRot(%d,%d)\n", dx, dy);
    send(sock, buf, strlen(buf), 0);
}

int setup_pi_connection(const char * device) {
    int fd = open(device, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        return -1;
    }

    struct termios tty;
    tcgetattr(fd, &tty);

    cfsetspeed(&tty, BAUDRATE);
    cfmakeraw(&tty);
    tty.c_cflag |= (CLOCAL | CREAD);
    tcsetattr(fd, TCSANOW, &tty);

    return fd;
}

int fruit_juice_test(int sock) {
    // move_player(sock, 235, 65, -1);
    put_block(sock, 235, 65, -1, "DIAMOND_BLOCK");
    return 1;
}

int main() {
    int sock = connect_to_fruitjuice(PLUGIN_IP, PLUGIN_PORT);
    if (sock < 0) {
        return 1;
    }
    printf("sock created\n");

    // fruit_juice_test(sock);
    // close(sock);
    // return 0;


    int pi_fd = setup_pi_connection(PI_PORT);
    if (pi_fd < 0) {
        printf("error pi_fd not created\n");
        return 1;
    }
    printf("pi_fd created");

    char buffer[256];
    int buf_idx = 0;
    char c;
    
    printf("connecting to pi \n");

    while (1) {
        int n = read(pi_fd, &c, 1);
        if (n <= 0) {
            continue;
        }
        if (c == '\n') {

            if (buf_idx > 0 && buffer[buf_idx - 1] == '\r') {
                buf_idx--;
            }

            buffer[buf_idx] = '\0';
            buf_idx = 0;

            char cmd[16];
            int x, y, z;
            char block[64];

            printf("%s\n", buffer);

            if (sscanf(buffer, "%15s %d %d %d %63s", cmd, &x, &y, &z, block) == 5) {
                if (strcmp(cmd, "BLOCK") == 0) {
                    put_block(sock, x, y, z, block);
                    printf("put block %d %d %d %s\n", x, y, z, block);
                }
            }
            else if (sscanf(buffer, "%15s %d %d %d", cmd, &x, &y, &z) == 4) {
                if (strcmp(cmd, "PLAYER") == 0) {
                    move_player(sock, x, y, z);
                    printf("move player %d %d %d\n", x, y, z);
                }
            }
            else if (sscanf(buffer, "%15s %d %d", cmd, &dx, &dy) == 3) {
                if (strcmp(cmd, "PLAYER_ROT") == 0) {
                    send_player_rotation(sock, dx, dy);
                    printf("send player rotation %d %d\n", dx, dy);
                }
            }

        } else {
            if (buf_idx < sizeof(buffer) - 1) {
                if (!(c == ' ' || c == '-' || (c >= '0' && c <= '9') ||
                    (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))) {
                    buffer[buf_idx++] = ' ';   // turn weird separators into spaces
                } else {
                buffer[buf_idx++] = c;
                }
            }
        }
    }
    // close(sock);
    return 0;
}