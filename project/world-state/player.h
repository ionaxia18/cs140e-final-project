#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include <stdbool.h>
#include "rpi.h"

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} pos_t;

typedef struct {
    int16_t x;
    int16_t y;
} p_rot_t;

struct player {
    uint16_t player_id;
    pos_t position;
    p_rot_t rotation;
};

typedef struct player player_t;

bool player_position_set(player_t* p, uint16_t x, uint16_t y, uint16_t z);

bool player_rotation_set(player_t* p, uint16_t x, uint16_t y);

#endif 

