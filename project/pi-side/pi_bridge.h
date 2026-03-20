
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

void do_move(player_t* player, pos_t new_pos);
void change_block(world_t* w, player_t* player, block_t block_selected);
world_t *initialize_server();


#endif