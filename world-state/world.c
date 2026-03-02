#include "world.h"


world_key_t world_make_key(world_pos_t p) {
    return (p.x << 20) | (p.y << 10) | p.z;
}


world_pos_t world_read_key(world_key_t k){
    world_pos_t p;
    uint32_t mask = 0x3ff;
    p.z = k & mask;
    p.y = (k >> 10) & mask;
    p.x = (k >> 20) & mask;
    return p;
}












