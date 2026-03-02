
#ifndef PI_BRIDGE_H
#define PI_BRIDGE_H

typedef struct coords {                                    
    int x;
    int y;
    int z;                    
} coords; // x y z coordinates

void uart_put_int(int val);
void do_move(char c);
void change_block(char c);


#endif