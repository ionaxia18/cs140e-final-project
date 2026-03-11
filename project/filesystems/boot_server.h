#ifndef __BOOT_SERVER_H__
#define __BOOT_SERVER_H__

#include "rpi.h"
#include "fat32.h"
#include "fat32-helpers.h"
#include "pi-sd.h"
#include "../../libpi/rpi.h"
#include "../constants.h"
#include "../world-state/world.h"
#include "../boot/world-gen.h"
#include "../world-state/pending.h"
#include "../world-state/hashtable.h"
#include "../world-state/player.h"

#define MAX_PENDING 1024
#define MAX_EDITS   2048

typedef struct __attribute__((packed)) {
    block_t block;
    pos_t pos;
    bool full;
} flat_entry_t;

typedef struct __attribute__((packed)) {
    player_t player;
    world_info_t info;
    uint32_t pending_size;
    uint32_t edits_size;

    flat_entry_t pending_blocks[MAX_PENDING];
    uint32_t pending_indices[MAX_PENDING];
    flat_entry_t edit_blocks[MAX_EDITS];
} file_t;

file_t * save_game(world_t * world, player_t *player, uint32_t * out_size);
void load_game(file_t *file_info, struct world *world, player_t *player);
void pi_to_plugin(world_t *world, player_t * player);
fat32_fs_t initialize_fs(pi_dirent_t * directory);
int create_boot_file(uint32_t seed, pi_dirent_t * directory, fat32_fs_t * fs);
int get_current_state(uint32_t seed,  pi_dirent_t * root, fat32_fs_t * fs, world_t * world, player_t * player);
int save_current_state(world_t * world, player_t * player, uint32_t seed, pi_dirent_t * directory, fat32_fs_t * fs);
int delete_boot_file(uint32_t seed, pi_dirent_t * directory, fat32_fs_t * fs);

#endif