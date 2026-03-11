#include "boot_server.h"
#include "player.h"
#include "world.h"
#include "../heap/allocator.h"

static char heap[64 * 1500];
static size_t heap_size = sizeof(heap);
static void* heap_start = heap;

void notmain() {
    // pi_dirent_t * directory = NULL;
    // fat32_fs_t fs = initialize_fs(directory);
    // delete_boot_file(0, directory, &fs);
    pi_dirent_t * directory = NULL;
    fat32_fs_t fs = initialize_fs(directory);
    myinit(heap_start, heap_size);
    // create_boot_file(0, directory, &fs);
    player_t *player = NULL;
    world_info_t info = {
        .seed = 0,
        .min = (pos_t){0, -60, 0},
        .max = (pos_t){16, -44, 16},
        .edits_cap = 2048,
        .pending_cap = 1024,
    };

    world_t* world = world_create(&info);
    get_current_state(0, directory, &fs, world, player);
    trace("got result\n");
    // pi_to_plugin(world, player);
    assert(player->player_id == 0);
    assert(player->position.x == 0);
    assert(player->position.y == -60);
    assert(player->position.z == 0);
    trace("got player with player_id %d, position at %d, %d, %d, and rotation at %d, %d\n", player->player_id, player->position.x, player->position.y, player->position.z, player->rotation.yaw, player->rotation.pitch);
    player_position_increment(player, 0, 1, 0);
    assert(player->position.x == 0);
    assert(player->position.y == -59);
    assert(player->position.z == 0);
    trace("finish player tests\n");

    pos_t pos = {.x = -50, .y = -50, .z =-50};
    trace("got block with %d\n", world_get_block(world, pos));
    world_set_block(world, pos, BLOCK_GRASS);
    block_t block = world_get_block(world, pos);
    trace("got block with %d\n", block);
    assert(world_get_block(world, pos) == BLOCK_GRASS);
    trace("finish world tests\n");
    save_current_state(world, player, 0, directory, &fs);
    //world_destroy(world);

    trace("tests passed\n");
}

