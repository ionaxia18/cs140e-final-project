#include "player.h"
#include "world.h"
#include "../pi-side/uart-helpers.h"
#include "../constants.h"

static int wrap_deg(int deg) {
    while (deg < 0) deg += 360;
    while (deg >= 360) deg -= 360;
    return deg;
}

float sin_deg(int deg) {
    deg = wrap_deg(deg);

    if (deg <= 90) return sin_q[deg];
    if (deg <= 180) return sin_q[180 - deg];
    if (deg <= 270) return -sin_q[deg - 180];
    return -sin_q[360 - deg];
}

float cos_deg(int deg) {
    return sin_deg(deg + 90);
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
static int16_t floorf_custom(float x) {
    int16_t i = (int16_t)x;
    if (x < i) i--;
    return i;
}

static float absf_custom(float x) {
    return x < 0.0f ? -x : x;
}

static pos_t block_pos(int x, int y, int z) {
    return (pos_t){(float)x, (float)y, (float)z};
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
/*
 * Returns:
 *   hit_block   = solid block the ray actually hit
 *   place_block = air block just before that solid block
 *
 * IMPORTANT:
 * If player.position is already the camera position, use eye_height = 0.0f.
 * If player.position is feet position, use eye_height = 1.62f.
 */
bool raycast_block(world_t *w, player_t *p, pos_t *hit_block, pos_t *place_block) {
    const float max_dist = 5.0f;

    const float eye_height = 1.62f;

    float ox = p->position.x;
    float oy = p->position.y + eye_height;
    float oz = p->position.z;

    float dx = -sin_deg(p->rotation.yaw) * cos_deg(p->rotation.pitch);
    float dy = -sin_deg(p->rotation.pitch);
    float dz =  cos_deg(p->rotation.yaw) * cos_deg(p->rotation.pitch);

    int x = floorf_custom(ox);
    int y = floorf_custom(oy);
    int z = floorf_custom(oz);

    int step_x = (dx > 0.0f) ? 1 : (dx < 0.0f ? -1 : 0);
    int step_y = (dy > 0.0f) ? 1 : (dy < 0.0f ? -1 : 0);
    int step_z = (dz > 0.0f) ? 1 : (dz < 0.0f ? -1 : 0);

    const float BIG = 1e30f;

    float t_delta_x = (step_x != 0) ? absf_custom(1.0f / dx) : BIG;
    float t_delta_y = (step_y != 0) ? absf_custom(1.0f / dy) : BIG;
    float t_delta_z = (step_z != 0) ? absf_custom(1.0f / dz) : BIG;

    float t_max_x = BIG;
    float t_max_y = BIG;
    float t_max_z = BIG;

    if (step_x > 0) t_max_x = ((float)(x + 1) - ox) / dx;
    else if (step_x < 0) t_max_x = (ox - (float)x) / (-dx);

    if (step_y > 0) t_max_y = ((float)(y + 1) - oy) / dy;
    else if (step_y < 0) t_max_y = (oy - (float)y) / (-dy);

    if (step_z > 0) t_max_z = ((float)(z + 1) - oz) / dz;
    else if (step_z < 0) t_max_z = (oz - (float)z) / (-dz);

    int last_air_x = x;
    int last_air_y = y;
    int last_air_z = z;

    float t = 0.0f;

    while (t <= max_dist) {
        if (t_max_x < t_max_y && t_max_x < t_max_z) {
            x += step_x;
            t = t_max_x;
            t_max_x += t_delta_x;
        } else if (t_max_y < t_max_z) {
            y += step_y;
            t = t_max_y;
            t_max_y += t_delta_y;
        } else {
            z += step_z;
            t = t_max_z;
            t_max_z += t_delta_z;
        }

        if (t > max_dist) {
            return false;
        }

        pos_t cur = block_pos(x, y, z);

        if (!world_pos_is_valid(cur)) {
            return false;
        }

        if (world_get_block(w, cur) != BLOCK_AIR) {
            *hit_block = cur;
            *place_block = block_pos(last_air_x, last_air_y, last_air_z);
            return true;
        }

        last_air_x = x;
        last_air_y = y;
        last_air_z = z;
    }

    return false;
}

bool valid_player_move(world_t *w, player_t* player, pos_t new_pos) {
    float dx = (new_pos.x - player->position.x);
    float dy = new_pos.y - player->position.y;
    float dz = new_pos.z - player->position.z;

    if (dx * dx > 1 || dy * dy > 1 || dz * dz > 1) {
        trace("displacement too far, cannot teleport= dx=%d, dy=%d, dz=%d\n", (int)dx, (int)dy, (int)dz);
        return false;
    }
    if (!world_pos_is_valid(new_pos)) {
        trace("new position is invalid, x=%d, y=%d, z=%d\n", (int)new_pos.x, (int)new_pos.y, (int)new_pos.z);
        return false;
    }
    pos_t lower = new_pos;
    lower.y += 1;
    pos_t upper = new_pos;
    upper.y += 1;
    if ((world_get_block(w, lower) != BLOCK_AIR) || (world_get_block(w, upper) != BLOCK_AIR)) {
        trace("block is type %d for lowe pos is %d %d %d\n", world_get_block(w, lower), (int)lower.x, (int)lower.y, (int)lower.z);
        trace("block is type %d for upper\n", world_get_block(w, upper));
        // world_print(w);
        uint32_t index = table_hash_index(lower, w->edits.cap);
        trace("hash for lower %d\n", index);
        if (w->edits.entries[index]) {
            trace("entry is valid\n");
            world_entry_t* cur = w->edits.entries[index];
            while (cur) {
                world_entry_t* next = cur->next;
                if (block_pos_equal(cur->pos, lower) || block_pos_equal(cur->pos, upper)) {
                      trace("entry block is %d\n", w->edits.entries[index]->block);
                    trace("entry pos is %d\n", w->edits.entries[index]->block);
                    trace("new position or block on top had a block already,  x=%d, y=%d, z=%d\nx=%d, y=%d, z=%d\n", (int)lower.x, (int)lower.y, (int)lower.z, (int)upper.x, (int)upper.y, (int)upper.z);
                    trace("block at lower %d, block at upper\n", world_get_block(w, lower), world_get_block(w, upper));
                }
            
                cur = next;
            }
        }
        // trace("entry block is %d\n", w->entries[index].block);


        return false;
    }
    return true;
}