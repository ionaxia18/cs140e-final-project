#include "rpi.h"
#include "../world-state/world.h"
#include "../world-state/hashtable.h"
#include "../../filesystems/pi-sd.h"
#include "../../filesystems/fat32.h"
#include "../pi-side/uart-helpers.h"


block_t world_base_block(const world_t* w, pos_t p);

// Query block in flat world with mountains
block_t make_flat_with_mtns(uint32_t seed, pos_t p);

// Query block in flat world
// Seed = 0
block_t flat_world(pos_t p);


// Create starting flat world with mountains
void create_world_file(char *name, fat32_fs_t *fs, pi_dirent_t *root);
void gen_flat_mtn_world(uint32_t seed);
void write_flat_mtn_world(uint32_t seed, world_t * world);