#include "rpi.h"
#include "world.h"
#include "hashtable.h"
#include "player.h"

uint32_t make_and_read_key(pos_t p) {
    world_key_t k = world_make_key(p);
    trace("Key: %d\n", k);
    pos_t p2 = world_read_key(k);
    trace("Pos: %d, %d, %d\n", p2.x, p2.y, p2.z);
    assert(p.x == p2.x && p.y == p2.y && p.z == p2.z);
    return k;
}

void notmain(void) {
    // range is {-512, 511} for each coord
    make_and_read_key((pos_t){0, 0, 0});
    make_and_read_key((pos_t){1, 2, 3});
    make_and_read_key((pos_t){-1, -2, -3});
    make_and_read_key((pos_t){1, -2, 3});
    make_and_read_key((pos_t){-1, 2, -3});
    make_and_read_key((pos_t){1, 2, -3});
    make_and_read_key((pos_t){-1, 2, 3});
    make_and_read_key((pos_t){500, -2, -200});

    // test max
    make_and_read_key((pos_t){511, 511, 511});
    make_and_read_key((pos_t){0, 511, 1});
    make_and_read_key((pos_t){0, 511, -1});

    // test min
    make_and_read_key((pos_t){0, 1, -512});
    make_and_read_key((pos_t){0, -512, 1});
    make_and_read_key((pos_t){0, -512, -1});
    make_and_read_key((pos_t){-512, -512, -512});
    trace("Key functions test passed\n");

}
