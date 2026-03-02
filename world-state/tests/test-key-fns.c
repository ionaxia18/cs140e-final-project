#include "rpi.h"
#include "world.h"

void notmain(void) {
    world_pos_t p = {0, 0, 0};
    world_key_t k = world_make_key(p);
    trace("Key: %d\n", k);
    p = world_read_key(k);
    trace("Pos: %d, %d, %d\n", p.x, p.y, p.z);
    assert(p.x == 0 && p.y == 0 && p.z == 0);
}
