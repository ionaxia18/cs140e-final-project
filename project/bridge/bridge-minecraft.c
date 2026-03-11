#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <termios.h>
#include <fcntl.h>
#include "../constants.h"
#include <netdb.h>
#define PLUGIN_IP "127.0.0.1"
#define PLUGIN_PORT 4711
#define BAUDRATE    B115200

typedef uint8_t block_t;

enum {
    BLOCK_AIR   = 0,
    BLOCK_STONE = 1,
    BLOCK_GRASS  = 2,
    BLOCK_DIRT = 3,
    BLOCK_COBBLESTONE = 4,
    BLOCK_WOOD = 5,
    BLOCK_WATER = 8
};

char *block_name(block_t b) {
    switch (b) {
        case BLOCK_AIR: return "AIR";
        case BLOCK_STONE: return "STONE";
        case BLOCK_GRASS: return "GRASS_BLOCK";
        case BLOCK_DIRT: return "DIRT";
        case BLOCK_COBBLESTONE: return "COBBLESTONE";
        case BLOCK_WOOD: return "OAK_LOG";
        case BLOCK_WATER: return "WATER";
        default: return "UNKNOWN";
    }
}

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
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }
    return sock;
}

int connect_to_fruitjuice_ngrok(const char *host, int port) {
    int sock;
    struct addrinfo hints, *res;

    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP

    if (getaddrinfo(host, port_str, &hints, &res) != 0) {
        perror("getaddrinfo");
        return -1;
    }

    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        freeaddrinfo(res);
        return -1;
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect");
        close(sock);
        freeaddrinfo(res);
        return -1;
    }

    freeaddrinfo(res);
    return sock;
}
void put_block(int sock, int x, int y, int z, char* block) {
    char buf[128];
    sprintf(buf, "world.setBlock(%d,%d,%d,%s)\n", x, y, z, block);
    send(sock, buf, strlen(buf), 0);
}

void move_player(int sock, float x, float y, float z) {
    char buf[128];
    sprintf(buf, "player.setPos(%f,%f,%f)\n", x, y, z);
    send(sock, buf, strlen(buf), 0);
}        

void send_player_rotation(int sock, int dx, int dy) {
    char buf[128];
    sprintf(buf, "player.setRotation(%d)\n", dx);
    send(sock, buf, strlen(buf), 0);
    sprintf(buf, "player.setPitch(%d)\n", dy);
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
    // put_block(sock, 235, 65, -1, "DIAMOND_BLOCK");
    put_block(sock, 235, 65, -1, 0);
    return 1;
}

int main() {
    // int sock = connect_to_fruitjuice(PLUGIN_IP, PLUGIN_PORT);
    int sock = connect_to_fruitjuice_ngrok("4.tcp.us-cal-1.ngrok.io", 19838);
    if (sock < 0) {
        return 1;
    }
    printf("sock created\n");

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
    char buf[128];
    sprintf(buf, "/setworldspawn 0 4 0");
    while (1) {
        int n = read(pi_fd, &c, 1);
        if (n <= 0) {
            continue;
        }
        if (c == '\n' || c == '\r') {

            if (buf_idx > 0 && (buffer[buf_idx - 1] == '\r' || buffer[buf_idx - 1] == '\t')) {
                buf_idx--;
            }

            buffer[buf_idx] = '\0';
            buf_idx = 0;

            char cmd[16];
            float x, y, z; 
            int rp, ry;
            int block = 0;
            printf("%s\n", buffer);

            if (sscanf(buffer, "%15s %f %f %f %d", cmd, &x, &y, &z, &block) == 5) {
                if (strcmp(cmd, "BLOCK") == 0) {
                    put_block(sock, x, y, z, block_name(block));
                    printf("put block %f %f %f %s\n", x, y, z, block_name(block));
                }
            }
            else if (sscanf(buffer, "%15s %f %f %f", cmd, &x, &y, &z) == 4) {
                if (strcmp(cmd, "PLAYER") == 0) {
                    move_player(sock, x, y, z);
                    printf("move player %f %f %f\n", x, y, z);
                }
            }
            else if (sscanf(buffer, "%15s %d %d", cmd, &rp, &ry) == 3) {
                if (strcmp(cmd, "ROT") == 0) {
                    send_player_rotation(sock, rp, ry);
                    printf("send player rotation %d %d\n", rp, ry);
                }
            }

        } else {
            if (buf_idx < sizeof(buffer) - 1) {
                if (!(c == ' ' || c == '-' || (c >= '0' && c <= '9') ||
                    (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '.'))) {
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