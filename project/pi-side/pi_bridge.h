
#ifndef PI_BRIDGE_H
#define PI_BRIDGE_H

#include <stdint.h>

typedef struct coords {                                    
    uint32_t  x;
    uint32_t y;
    uint32_t z;                    
} coords; // x y z coordinates

coords do_move(char c);

#endif