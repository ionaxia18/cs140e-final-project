
#include "world-gen.h"
#include "constants.h"
// Deterministic hash: same inputs produce same output
static uint32_t hash2d(uint32_t seed, int16_t x, int16_t z) {
    uint32_t h = seed;
    h ^= (uint32_t)(uint16_t)x * 374761393u;
    h ^= (uint32_t)(uint16_t)z * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

// Query block in flat world with mountains
block_t make_flat_with_mtns(uint32_t seed, pos_t p) {
    int base_height = -60;
    // hash gives 0-15, make a mountain if hash < 2 
    uint32_t h = hash2d(seed, p.x, p.z);
    int mtn_height = (h & 0xF) < 2 ? (int)((h >> 4) & 0x7) : 0;

    if (mtn_height == 0)
        return flat_world(p);

    int top = base_height + mtn_height;
    if (p.y > top)         return BLOCK_AIR;
    if (p.y == top)        return BLOCK_STONE;
    if (p.y > base_height) return BLOCK_STONE;
    return flat_world(p);
}

// Query block in flat world
// Seed = 0
block_t flat_world(pos_t p) {
    int16_t y = p.y;
    if (y == -60) {
        return BLOCK_GRASS;
    } else if (y < -60 && y > -63) {
        return BLOCK_DIRT;
    } else if (y <= -63 && y > -65) {
        return BLOCK_STONE;
    } else {
        return BLOCK_AIR;
    }
}

block_t world_base_block(const world_t* w, pos_t p) {
    const world_info_t* info = w->info;
    if (info->seed == 0) {
        return flat_world(p);
    } else {
   
        return make_flat_with_mtns(info ->seed, p);
    }
}


void create_world_file(char *name, fat32_fs_t *fs, pi_dirent_t *root) {
  kmalloc_init_mb(FAT32_HEAP_MB);
  pi_sd_init();

  trace("Reading the MBR\n");
  mbr_t *mbr = mbr_read();
  trace("Loading the first partition\n");
  mbr_partition_ent_t partition;
  memcpy(&partition, mbr->part_tab1, sizeof(mbr_partition_ent_t));
  assert(mbr_part_is_fat32(partition.part_type));

  trace("Loading the FAT\n");
  *fs = fat32_mk(&partition);

  *root = fat32_get_root(fs);
  trace("Loading the root directory\n");
  pi_directory_t dir = fat32_readdir(fs, root);

  printk("Creating flat mountain world\n");
  // uint32 is max 10 digits

  trace("Creating the world file\n");
  pi_dirent_t *world_file = fat32_create(fs, root, name, 0);
  assert(world_file != NULL);
}

#define WORLD_SLICE_Y 2
#define WORLD_SLICE_X 2
#define WORLD_SLICE_Z 2
#define LINE_MAX 20
#define WORLD_SZ (WORLD_SLICE_Y * WORLD_SLICE_X * WORLD_SLICE_Z)

void write_flat_mtn_world(uint32_t seed, world_t * world) {
    printk("Writing initial world for seed %u\n", seed);

    // One contiguous buffer: WORLD_LINES lines, each up to LINE_MAX bytes
    char data[LINE_MAX];
    size_t off = 0;

    for (int y = -60; y > -60 - WORLD_SLICE_Y; y--) {
        for (int x = 0; x < WORLD_SLICE_X; x++) {
            for (int z = 0; z < WORLD_SLICE_Z; z++) {
                pos_t p = (pos_t) {x, y, z};
                block_t block = make_flat_with_mtns(seed, p);
                if (block == BLOCK_STONE) {
                    world_set_block(world, p, block);
                }
                // if (block == BLOCK_STONE) {
                //     snprintk(data + off, LINE_MAX, "%d %d %d %d\n", x, y, z, block);
                //     trace("Block %d at %d, %d, %d\n", block, x, y, z);
                //     uart_put_str(data);
                // }
            }
        }
    }
}
static const glyph_t *find_glyph(char c) {
    for (unsigned i = 0; i < sizeof(font)/sizeof(font[0]); i++) {
        if (font[i].c == c)
            return &font[i];
    }
    return 0;
}
static void draw_char(world_t *world, char c, int start_x, int base_y, int z, block_t block) {
    const glyph_t *g = find_glyph(c);
    if (!g) return;

    for (int row = 0; row < FONT_H; row++) {
        for (int col = 0; col < FONT_W; col++) {
            if (g->rows[row] & (1 << (FONT_W - 1 - col))) {
                pos_t p = {
                    .x = start_x + col,
                    .y = base_y + (FONT_H - 1 - row),
                    .z = z
                };
                world_set_block(world, p, block);
            }
        }
    }
}

static void draw_text(world_t *world, const char *s, int start_x, int base_y, int z, block_t block) {
    int x = start_x;
    while (*s) {
        draw_char(world, *s, x, base_y, z, block);
        x += FONT_W + 1;   // 1 column spacing
        s++;
    }
}

void write_demo_world(uint32_t seed, world_t* world) {
        // Write text near spawn (0, -60, 0)
    // Raised slightly above ground so it is visible.
    draw_text(world, "WELCOME TO", 2,  -54, 8, BLOCK_GLOWSTONE);
    draw_text(world, "PICRAFT",    12, -61 + 8, 8, BLOCK_GLOWSTONE);
}
// void gen_flat_mtn_world(uint32_t seed) {
//     char name[20];
//     snprintk(name, sizeof(name), "%u", seed);
//     strcat(name, ".TXT");
//     fat32_fs_t fs;
//     pi_dirent_t root;

//     create_world_file(name, &fs, &root);

//     write_flat_mtn_world(seed, &fs, &root, name);
//     printk("Check your SD card for a file called %s\n", name);

// }

