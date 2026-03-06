#include "player.h"
#include "world.h"

bool player_position_set(player_t* p, uint16_t x, uint16_t y, uint16_t z) {
    pos_t new_pos = (pos_t){x, y, z};
    if (!world_pos_is_valid(new_pos)) { return false; }
    p->position = new_pos;
    return true;
}

bool player_rotation_set(player_t* p, uint16_t yaw, uint16_t pitch) {
    p_rot_t new_rot = (p_rot_t){x, y};
    p->rotation = new_rot;
    return true;
}

bool player_rotation_increment(player_t* p, uint16_t yaw, uint16_t pitch) {
    p_rot_t new_rot = (p_rot_t){p->rotation.x + dx, p->rotation.y + dy};
    p->rotation = new_rot;
    return true;
}

bool player_position_increment(player_t* p, uint16_t dx, uint16_t dy, uint16_t dz) {
    pos_t new_pos = (pos_t){p->position.x + dx, p->position.y + dy, p->position.z + dz};
    p->position = new_pos;
    return true;
}