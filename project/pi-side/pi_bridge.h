
#ifndef PI_BRIDGE_H
#define PI_BRIDGE_H

#include "../../libpi/rpi.h"
#include "../constants.h"

#include "../world-state/world.h"
#include "../world-state/pending.h"
#include "../world-state/hashtable.h"
#include "../world-state/player.h"

#include "../boot/world-gen.h"

#include "../gpio/arcade.h"
#include "../gpio/joystick.h"
#include "../gpio/matrix.h"


#include "uart-helpers.h"
#include "../filesystems/boot_server.h"
#include "../heap/allocator.h"

typedef struct coords {                                    
    int x;
    int y;
    int z;                    
} coords; // x y z coordinates

void uart_put_int(int val);
void uart_put_str(char* str);

void do_move(player_t* player, pos_t new_pos);
void change_block(world_t* w, player_t* player, block_t block_selected);
void update_rotation(player_t* player, uint16_t yaw, uint16_t pitch);
world_t *initialize_server();


#endif