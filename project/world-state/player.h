#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include <stdbool.h>
#include "rpi.h"
#include "world.h"
#include "world.h"
#include "hashtable.h"

typedef struct {
    int16_t yaw;
    int16_t pitch;
} p_rot_t;

struct player {
    uint16_t player_id;
    pos_t position;
    p_rot_t rotation;
};

float sin_deg(int deg);
float cos_deg(int deg);

// int16_t floorf_custom(float x);

typedef struct player player_t;

bool player_position_set(player_t* p, float x, float y, float z);

bool player_position_increment(player_t* p, float dx, float dy, float dz);
bool player_rotation_increment(player_t* player, int16_t yaw, int16_t pitch);
bool rotation_increment(p_rot_t* rot, int16_t yaw, int16_t pitch); 
bool player_rotation_set(player_t* p, int16_t x, int16_t y);

bool pointing_block(world_t* w, player_t* p, pos_t* out_pos);

bool raycast_block(world_t *w, player_t *p, pos_t *hit_block, pos_t *place_block);
bool valid_player_move(world_t *w, player_t* player, pos_t new_pos) ;

#endif 

