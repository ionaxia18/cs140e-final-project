#include "world.h"

world_key_t world_make_key(world_pos_t p) {
    uint32_t mask = 0x3ff; 
    // mask to get rid of sign bit
    return ((p.x & mask) << 20) | ((p.y & mask) << 10) | (p.z & mask);
}

world_pos_t world_read_key(world_key_t k){
    world_pos_t p; 
    uint32_t mask = 0x3ff;
    uint32_t masked_z = k & mask;
    uint32_t masked_y = (k >> 10) & mask;
    uint32_t masked_x = (k >> 20) & mask;

    // check if x, y, and z were negative via sign bit and fill in if so
    p.x = (masked_x & 0x200) ? (masked_x | 0xFC00) : masked_x;
    p.y = (masked_y & 0x200) ? (masked_y | 0xFC00) : masked_y;
    p.z = (masked_z & 0x200) ? (masked_z | 0xFC00) : masked_z;

    return p;
}













