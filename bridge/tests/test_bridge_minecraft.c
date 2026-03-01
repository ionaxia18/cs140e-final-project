#include "bridge-minecraft.h"
#include <stdio.h>

int main()
{
    int mc = connect_to_fruitjuice("127.0.0.1", 4711);
    if (mc < 0) {
        printf("failed to connect\n");
        return 1;
    }
    printf("connected to fruitjuice\n");

    put_block(mc,0,4,0,"stone");
    printf("put stone block\n");

    put_block(mc,1,4,0,"gold_block");
    printf("put gold block\n");

    remove_block(mc,1,4,0);
    printf("put remove gold block\n");

    move_player(mc,2,4,2);
    printf("move player\n");
    
    printf("Test complete\n");
    fflush(stdout);
    return 0;
}