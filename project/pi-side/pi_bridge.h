
#ifndef PI_BRIDGE_H
#define PI_BRIDGE_H

#include "../world-state/world.h"
#include "../world-state/world-gen.h"

typedef struct coords {                                    
    int x;
    int y;
    int z;                    
} coords; // x y z coordinates

void uart_put_int(int val);
void do_move(world_t* w, char c);
void change_block(char c);


#endif