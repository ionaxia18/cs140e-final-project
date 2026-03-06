#include "player.h"
#include "world.h"
#include "../pi-side/uart-helpers.h"

float sin_deg(float deg) {
    float x = deg * 3.14159265 / 180.0f;

    float x2 = x * x;
    return x * (1 - x2/6 + x2*x2/120);
}

float cos_deg(float deg) {
    return sin_deg(deg + 90.0f);
}

bool player_position_set(player_t* p, int16_t x, int16_t y, int16_t z) {
    pos_t new_pos = (pos_t){x, y, z};
    if (!world_pos_is_valid(new_pos)) { return false; }
    p->position = new_pos;
    return true;
}

bool player_rotation_set(player_t* p, int16_t yaw, int16_t pitch) {
    p_rot_t new_rot = (p_rot_t){yaw, pitch};
    p->rotation = new_rot;
    return true;
}

bool player_rotation_increment(player_t* p, int16_t yaw, int16_t pitch) {
    int16_t new_yaw = p->rotation.yaw + yaw;
    int16_t new_pitch = p->rotation.pitch + pitch;
    if (new_yaw > 360) {
        new_yaw -= 360;
    }
    if (new_yaw < 0) {
        new_yaw += 360;
    }
    if (new_pitch > 90) {
        new_pitch = 90;
    }
    if (new_pitch < -90) {
        new_pitch = -90;
    }
    p_rot_t new_rot = (p_rot_t){new_yaw, new_pitch};
    p->rotation = new_rot;
    return true;
}

bool player_position_increment(player_t* p, int16_t dx, int16_t dy, int16_t dz) {
    pos_t new_pos = (pos_t){p->position.x + dx, p->position.y + dy, p->position.z + dz};
    p->position = new_pos;
    return true;
}

block_t pointing_block(world_t* w, player_t* p, pos_t* block_pos) {
    pos_t pos = p->position;
    p_rot_t rot = p->rotation;
    float dx = -sin_deg(rot.yaw) * cos_deg(rot.pitch);
    float dy = sin_deg(rot.pitch);
    float dz = cos_deg(rot.yaw) * cos_deg(rot.pitch);
    int16_t max_distance = 2;
    float step_size = 0.5;
    for (int i = 0; i < max_distance; i++) {
        pos_t new_pos = (pos_t){(int16_t)(pos.x + dx * step_size), (int16_t)(pos.y + dy * step_size), (int16_t)(pos.z + dz * step_size)};
        if (!world_pos_is_valid(new_pos)) { return BLOCK_AIR; }
        if (world_get_block(w, new_pos) != BLOCK_AIR) { 
            *block_pos = new_pos;
            return world_get_block(w, new_pos); 
        }
    }
    return BLOCK_AIR;
}