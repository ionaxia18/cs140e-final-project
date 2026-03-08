#ifndef BRIDGE_MINECRAFT_H
#define BRIDGE_MINECRAFT_H

int connect_to_fruitjuice(const char * ip, int port);
int setup_pi_connection(const char * device);
void put_block(int sock, int x, int y, int z, block_t block);
void move_player(int sock, int x, int y, int z);
int fruit_juice_test(int sock);
#endif