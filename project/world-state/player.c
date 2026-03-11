#include "player.h"
#include "world.h"
#include "../pi-side/uart-helpers.h"

float sin_deg(int deg) {
    while (deg > 180.0) deg -= 360.0;
    while (deg < -180.0) deg += 360.0;

    float x = deg * 3.14159265f / 180.0f;

    float x2 = x * x;
    return x * (1 - x2/6 + x2*x2/120);
}

float cos_deg(int deg) {
    return sin_deg(deg + 90.0f);
}

int16_t floorf_custom(float x) {
    int16_t i = (int16_t)x;
    if (x < i) i--;
    return i;
}

bool player_position_set(player_t* p, float x, float y, float z) {
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

bool player_rotation_increment(player_t* player, int16_t yaw, int16_t pitch) {
    return rotation_increment(&player->rotation, yaw, pitch);
}
bool rotation_increment(p_rot_t* rot, int16_t yaw, int16_t pitch) {
    int16_t new_yaw = rot->yaw + yaw;
    int16_t new_pitch = rot->pitch + pitch;
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
    rot->yaw = new_yaw;
    rot->pitch = new_pitch;
    return true;
}
bool player_position_increment(player_t* p, float dx, float dy, float dz) {
    pos_t new_pos = (pos_t){p->position.x + dx, p->position.y + dy, p->position.z + dz};
    p->position = new_pos;
    return true;
}


pos_t pointing_block(world_t* w, player_t* p) {
    pos_t pos = p->position;
    // pos_t new_pos = pos;
    p_rot_t rot = p->rotation;
    float start_x = pos.x + 0.5f;
    float start_y = pos.y + 1.6f;
    float start_z = pos.z + 0.5f;
    float dx = -sin_deg(rot.yaw) * cos_deg(rot.pitch);
    float dy = -sin_deg(rot.pitch);
    float dz = cos_deg(rot.yaw) * cos_deg(rot.pitch);
    int16_t max_distance = 2;
    pos_t last_pos = (pos_t){
        floorf_custom(start_x),
        floorf_custom(start_y),
        floorf_custom(start_z)
    };
    pos_t prev_checked = last_pos;
    // float step_size = 0.1;
    // pos_t last_pos = pos;
    //  trace("current difference is %d, %d, %d", (int16_t)(dx * 10), (int16_t)(dy * 10), (int16_t)(dz * 10));
    
    for (float i = 0; i < max_distance; i+= 0.05) {
        pos_t new_pos = (pos_t){
            floorf_custom(start_x + dx * i),
            floorf_custom(start_y + dy * i),
            floorf_custom(start_z + dz * i)
        };
        if (new_pos.x == prev_checked.x &&
            new_pos.y == prev_checked.y &&
            new_pos.z == prev_checked.z)
            continue;
        prev_checked = new_pos;

        if (!world_pos_is_valid(new_pos)) {
            return last_pos;
        }
        if (world_get_block(w, new_pos) != BLOCK_AIR) {
            return last_pos;
        }

        last_pos = new_pos;
    }

    return last_pos;
}
