#include "../world-gen.h"



void notmain(void) {
    // gen_flat_mtns(1);
    // trace("PASS: %s\n", __FILE__);
    // gen_flat_mtn_world(6);
    write_flat_mtn_world(6, NULL, NULL, NULL);
    trace("PASS: %s\n", __FILE__);
}