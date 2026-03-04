#include "player.h"
#include "world.h"

bool player_position_set(player_t* p, uint16_t x, uint16_t y, uint16_t z) {
    pos_t new_pos = (pos_t){x, y, z};
    if (!world_pos_is_valid(new_pos)) { return false; }
    p->position = new_pos;
    return true;
}

bool player_rotation_set(player_t* p, uint16_t x, uint16_t y) {
    p_rot_t new_rot = (p_rot_t){x, y};
    p->rotation = new_rot;
    return true;
}

