#include "rpi.h"
#include "world.h"
#include "hashtable.h"

world_t *init_world() {
    world_info_t info = {
        .seed        = 0,
        .min         = (pos_t){0, 0, 0},
        .max         = (pos_t){4, 4, 4},
        .edits_cap   = 16, 
        .pending_cap = 16,
    };

    player_t player = {
        .player_id = 0,
        .position  = (pos_t){0, 0, 0},
        .rotation  = (p_rot_t){0, 0},
    };

    world_t *w = world_create(&info, &player);
    assert(w != NULL);
    return w;
}

void test_delete_entry() {
    trace("test_delete_entry\n");
    world_t *w = init_world();

    pos_t p1 = (pos_t){0, 0, 0};
    pos_t p2 = (pos_t){1, 0, 0};
    pos_t p3 = (pos_t){2, 0, 0};

    assert(world_set_block(w, p1, BLOCK_GRASS));
    assert(world_set_block(w, p2, BLOCK_DIRT));
    assert(world_set_block(w, p3, BLOCK_STONE));
    assert(w->edits.size == 3);

    assert(table_get_entry(&w->edits, p1) != NULL);
    assert(table_get_entry(&w->edits, p2) != NULL);
    assert(table_get_entry(&w->edits, p3) != NULL);

    assert(table_delete_entry(&w->edits, p2));
    assert(w->edits.size == 2);
    assert(table_get_entry(&w->edits, p2) == NULL);

    assert(table_get_entry(&w->edits, p1) != NULL);
    assert(table_get_entry(&w->edits, p3) != NULL);

    assert(!table_delete_entry(&w->edits, p2));
    assert(w->edits.size == 2);

    world_destroy(w);
    trace("all delete tests passed\n");
}

void test_world_destroy_reuse_heap() {
    trace("all destroy tests passed\n");

    world_t *w1 = init_world();
    for (int i = 0; i < 5; i++) {
        pos_t p = (pos_t){i, 0, 0};
        assert(world_set_block(w1, p, BLOCK_GRASS));
    }
    assert(w1->edits.size == 5);
    world_destroy(w1);

    world_t *w2 = init_world();
    for (int i = 0; i < 5; i++) {
        pos_t p = (pos_t){i, 0, 0};
        assert(world_set_block(w2, p, BLOCK_STONE));
        world_entry_t *e = table_get_entry(&w2->edits, p);
        assert(e != NULL && e->block == BLOCK_STONE);
    }
    assert(w2->edits.size == 5);
    world_destroy(w2);

    trace("all destroy tests passed\n");
}

void notmain() {
    test_delete_entry();
    test_world_destroy_reuse_heap();
    trace("all tests passed\n");
}