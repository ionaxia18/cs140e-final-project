#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include <stdbool.h>
#include "rpi.h"
#include "world.h"
#include "world.h"

typedef struct {
    int16_t yaw;
    int16_t pitch;
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

typedef struct player player_t;

bool player_position_set(player_t* p, int16_t x, int16_t y, int16_t z);

bool player_position_increment(player_t* p, int16_t dx, int16_t dy, int16_t dz);

bool player_rotation_increment(player_t* p, int16_t yaw, int16_t pitch);
bool player_position_set(player_t* p, int16_t x, int16_t y, int16_t z);

bool player_position_increment(player_t* p, int16_t dx, int16_t dy, int16_t dz);

bool player_rotation_increment(player_t* p, int16_t yaw, int16_t pitch);

bool player_rotation_set(player_t* p, int16_t x, int16_t y);

pos_t pointing_block(world_t* w, player_t* p);

#endif 

