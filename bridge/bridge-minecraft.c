#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>


// setup tcp function so that it can be used to talk to fruitjuice plugin
int connect_to_fruitjuice(const char * ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
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

void remove_block(int sock, int x, int y, int z) {
    char buf[128];
    sprintf(buf, "world.setBlock(%d, %d, %d, air)\n", x, y, z);
    send(sock, buf, strlen(buf), 0);
}

void put_block(int sock, int x, int y, int z, const char * block) {
    char buf[128];
    sprintf(buf, "world.setBlock(%d, %d, %d, %s)\n", x, y, z, block);
    send(sock, buf, strlen(buf), 0);
}

void move_player(int sock, int x, int y, int z) {
    char buf[128];
    sprintf(buf, "player.setTile(%d, %d, %d)\n", x, y, z);
    send(sock, buf, strlen(buf), 0);
}
