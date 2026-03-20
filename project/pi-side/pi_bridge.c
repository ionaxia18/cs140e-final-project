#include "pi_bridge.h"

static char heap[64 * 1500];
static size_t heap_size = sizeof(heap);
static void* heap_start = heap;
#define BAUDRATE B115200

// Attempts one interaction edit from the player's current camera direction.
// - If block_selected == BLOCK_AIR, this breaks the first solid block hit.
// - Otherwise, it places block_selected in the nearest air cell before the hit.
// The function rejects edits that would place into the player's body and only
// sends UART output after world_set_block succeeds.
void change_block(world_t* w, player_t* player, block_t block_selected) {
    pos_t hit;
    pos_t place;

    if (!raycast_block(w, player, &hit, &place)) {
        return;
    }

    trace("hit=(%d,%d,%d) block=%d place=(%d,%d,%d)\n",
          (int)hit.x, (int)hit.y, (int)hit.z,
          world_get_block(w, hit),
          (int)place.x, (int)place.y, (int)place.z);

    /* For breaking: delete block at hit. For placing: add block at place. */
    pos_t target = (block_selected == BLOCK_AIR) ? hit : place;
    pos_t pos = player->position;
    pos_t lower = {floorf_custom(pos.x), floorf_custom(pos.y), floorf_custom(pos.z)};
    pos_t upper = lower;
    upper.y += 1;
    pos_t target_floor = {floorf_custom(target.x), floorf_custom(target.y), floorf_custom(target.z)};
    if (block_pos_equal(target_floor, upper) || block_pos_equal(target_floor, lower)) {
        trace("cant place block in player");
        return;
    }
    if (!world_set_block(w, target, block_selected)) {
        trace("world_set_block FAILED at (%d,%d,%d)\n",
                (int)target.x, (int)target.y, (int)target.z);
        return;
    }
    // send whatever update happened over uart to bridge
    send_set_block(target, block_selected);
}

// Creates the in-memory world state used by the main control loop.
// Bounds and table capacities are centralized here so tuning world size or
// memory footprint stays in one place.
world_t* initialize_server() {
    /* static so it outlives this function - world->info points to it */
    static world_info_t info = {
        // 0 for flatworld
        .seed = 0,
        .min = (pos_t){-32, -59, -32},
        .max = (pos_t){32, -44, 32},
        .edits_cap = 2048,
        .pending_cap = 1024,
    };

    world_t* w = world_create(&info);
    if (!w) {
        panic("Failed to create world");
        return w;
    }
    return w;
}

// Returns true when yaw or pitch changed since the previous loop iteration.
// Used to avoid spamming ROT messages when the camera is stationary.
bool rotation_changed(p_rot_t cur, p_rot_t old) {
    return (cur.pitch != old.pitch) || (cur.yaw != old.yaw);
}

// Pi program entrypoint.
// Initializes allocator/world/hardware, then runs the control loop that:
// 1) reads movement + camera input,
// 2) applies movement/collision + block edits,
// 3) streams PLAYER/ROT/BLOCK messages to the host bridge over UART.
// Keypad 15 restores saved world; keypad 16 saves and exits.
void notmain() {
    player_t player = {
        .player_id = 0,
        .position = (pos_t){0, -59, 0},
        .rotation = (p_rot_t){0, 0}
    };
    myinit(heap_start, heap_size);
    world_t* w = initialize_server();

    pi_dirent_t * directory = NULL;
    fat32_fs_t fs = initialize_fs(directory);

    // initialize hardware components and uart
    matrix_init();
    arcade_init();
    joystick_init();
    uart_init();

    pos_t new_pos = player.position;
    pos_t fall_pos = player.position;
    p_rot_t last_rot = player.rotation;
    block_t block_selected = 0;
    block_t last_block = 0;  /* for debounce: only place on button press, not hold */
    int last_destroy = 0;      /* for debounce: only break on button press, not hold */
    trace("Server initialized\n");
    send_player_move(&player);
    send_player_rotation(&player);
    uart_flush_tx();
    world_print(w);
    trace("Welcome to Picraft! Ctrl+C to start playing.\n");

    // begin pulling from gpio to update world
    while (1) {
        pos_t *displacement = &(pos_t){0, 0, 0};
        int destroy = read_move_joystick(displacement);
        int place = read_camera_joystick(&player.rotation);

        // if player actually moved
        if (displacement->x || displacement->y || displacement->z) {
            new_pos = player_next_move(&player, *displacement);
            fall_pos = new_pos;
            fall_pos.y -= 1;
            // check if player can fall downwards
            if (valid_player_move(w, &player, fall_pos) && world_get_block(w, fall_pos) == BLOCK_AIR) {
                new_pos.y -= 1;
            }
            if (valid_player_move(w, &player, new_pos)) {
                player.position = new_pos;
                send_player_move(&player);
            } else {
                // if cant move into block, check if player can jump ontop of block
                new_pos.y += 1;
                if (valid_player_move(w, &player, new_pos)) {
                    player.position = new_pos;
                    send_player_move(&player);
                }
            }
        }

        if (rotation_changed(player.rotation, last_rot)) {
            send_player_rotation(&player);
            last_rot = player.rotation;
        }

        block_t new_block = read_block();
        if (new_block != 0) {
            block_selected = new_block;
        }
        if (destroy && !last_destroy) {
            change_block(w, &player, BLOCK_AIR);
        } else if (block_selected && place && block_selected != 16 && block_selected != 15 && !last_block) {
            /* Only place on press (rising edge), not while held, and if block_selected is valid*/
            // updates w as well
            change_block(w, &player, block_selected);
            last_block = block_selected;
        } else {
            last_block = BLOCK_AIR;
        }
        last_destroy = destroy;
        // button 16 saves world and quits program
        if (block_selected == 16 && !last_block) {
            save_current_state(w, &player, 0, directory, &fs);
            world_print(w);
            world_destroy(w);
            uart_put_str("DONE\n");
            return;
        } else if (block_selected == 15 && !last_block) {
            // button 15 reads saved world from file and sends to fruitjuice to reset world
            get_current_state(0, directory, &fs, w, &player);
            last_block = BLOCK_AIR;
            block_selected = BLOCK_AIR;
        }
        delay_ms(75);
        uart_flush_tx();
    }
}