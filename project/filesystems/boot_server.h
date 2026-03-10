#ifndef __BOOT_SERVER_H__
#define __BOOT_SERVER_H__

#include "rpi.h"
#include "fat32.h"
#include "fat32-helpers.h"
#include "pi-sd.h"
#include "../../libpi/rpi.h"
#include "../constants.h"
#include "../world-state/world.h"
#include "../world-state/world-gen.h"
#include "../world-state/pending.h"
#include "../world-state/hashtable.h"
#include "../world-state/player.h"

typedef struct file {
    world_t world;
    player_t player;
} file_t;

bool boot_file_exists(void);
int create_boot_file(void);
file_t *get_current_state(void);
int save_current_state(file_t *file_info);
int delete_boot_file(void);

#endif