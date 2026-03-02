
#ifndef PI_BRIDGE_H
#define PI_BRIDGE_H

typedef struct coords {                                    
    int x;
    int y;
    int z;                    
} coords; // x y z coordinates

coords do_move(char c);

#endif