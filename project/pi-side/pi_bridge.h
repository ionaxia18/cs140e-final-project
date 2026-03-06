
#ifndef PI_BRIDGE_H
#define PI_BRIDGE_H

#include "../../libpi/rpi.h"
#include "../constants.h"
#include "../world-state/world.h"
#include "../world-state/world-gen.h"
#include "../world-state/pending.h"
#include "../world-state/hashtable.h"
#include "../world-state/player.h"

typedef struct coords {                                    
    int x;
    int y;
    int z;                    
} coords; // x y z coordinates

void uart_put_int(int val);
void uart_put_str(char* str);

void do_move(char c, player_t* player);
void change_block(char c, world_t* w, player_t* player);
void update_rotation(player_t* player, uint16_t yaw, uint16_t pitch);
world_t *initialize_server();


#endif