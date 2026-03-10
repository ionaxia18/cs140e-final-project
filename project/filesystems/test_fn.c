#include "boot_server.h"
#include "player.h"
#include "world.h"

void notmain() {
    // create_boot_file();
    file_t * result = get_current_state();
    trace("got result\n");
    player_t player = result->player;
    world_t world = result->world;
    assert(result->player.player_id == 0);
    assert(result->player.position.x == 0);
    assert(result->player.position.y == -60);
    assert(result->player.position.z == 0);
    trace("got player with player_id %d, position at %d, %d, %d, and rotation at %d, %d", player.player_id, player.position.x, player.position.y, player.position.z, player.rotation.yaw, player.rotation.pitch);
    player_position_increment(&player, 0, 1, 0);
    assert(player.position.x == 0);
    assert(player.position.y == -59);
    assert(player.position.z == 0);
    trace("finish player tests\n");

    // world_info_t * info = world.info;
    // world_table_t edits = world.edits;
    // pending_table_t pending = world.pending;
    pos_t pos = {.x = 0, .y = 0, .z =0};
    world_set_block(&world, pos, BLOCK_STONE);
    block_t block = world_get_block(&world, pos);
    trace("got block with %d\n", block);
    assert(world_get_block(&world, pos) == BLOCK_STONE);
    trace("finish world tests\n");


    trace("tests passed\n");
}

// player_t player = {.player_id = 0,
//         .position = (pos_t) {0, -60, 0},
//         .rotation = (p_rot_t) {0, 0}
//     };
