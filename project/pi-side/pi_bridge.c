#include "pi_bridge.h"
#include "../../libpi/rpi.h"
#include "../constants.h"
#include "../world-state/world.h"
#include "../world-state/world-gen.h"
#include "../world-state/pending.h"
#include "../world-state/hashtable.h"
#include "../world-state/player.h"
#define BAUDRATE B115200

// struct coords p_coords = {235, 65, -1};
// char * block = "STONE";

void uart_put_int(int val) {
    char buf[12];              // enough for -2147483648 + '\0'
    int i = 0;

    if (val == 0) {
        uart_put8('0');
        return;
    }

    if (val < 0) {
        uart_put8('-');
        // avoid overflow on INT_MIN if you care:
        // unsigned u = (unsigned)(-(val+1)) + 1;
        val = -val;
    }

    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }

    while (i--) {
        uart_put8(buf[i]);
    }
}


void uart_put_str(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        uart_put8(str[i]);
    }
}

// takes in a character move, will move the player on pi side and return new coordinates
void do_move(world_t* w, char c, player_t* player) {
    if (c == 'w') {
        player_position_increment(player, 0, 1, 0);
    } else if (c == 'a') {
        player_position_increment(player, -1, 0, 0);
    } else if (c == 's') {
        player_position_increment(player, 0, -1, 0);
    } else if (c == 'd') {
        player_position_increment(player, 1, 0, 0);
    }

    if (!world_pos_is_valid(player->position)) {
        panic("invalid move to position %d %d %d", player->position.x, player->position.y, player->position.z);
    } 
    
    uart_put_str("PLAYER ");
    uart_put_int(player->position.x);
    uart_put_str(" ");
    uart_put_int(player->position.y);
    uart_put_str(" ");
    uart_put_int(player->position.z);
    uart_put_str("\n");
 }

void change_block(char c, block_t block, pos_t pos) {
    // this depends on how we update the player rotation ?
    char * cur_block = "";
    if (c == 'p') {
        cur_block = block;
        world_set_block(w, pos, block);
    }
    else if (c == 'r') {
        cur_block = "AIR";
        world_set_block(w, pos, BLOCK_AIR);
    }
    const char *str = "BLOCK";
    for (size_t i = 0; str[i] != '\0'; i++) {
        uart_put8((uint8_t) str[i]);
    }
    uart_put8(' ');
    uart_put_int(pos.x);
    uart_put_int(pos.y);
    uart_put_int(pos.z);
    for (size_t i = 0; cur_block[i] != '\0'; i++) {
        uart_put8((uint8_t) cur_block[i]);
    }
    uart_put8('\r\n');
}
 
void update_rotation(world_t* w, uint16_t yaw, uint16_t pitch) {
    player_t* player = w->player;
    player_rotation_increment(player, yaw, pitch);
    // idk how to scale the rotation
    uart_put_str("ROT ");
    uart_put_int(yaw);
    uart_put_str(" ");
    uart_put_int(pitch);
    uart_put_str("\n");
}

void initialize_server() {
    // initialize world seed
    world_info_t info = {
        .seed = 0,
        .min = (pos_t){0, -60, 0},
        .max = (pos_t){16, -44, 16},
        .edits_cap = 4096,
        .pending_cap = 4096,
    };

    world_t* w = world_create(&info, &player);
    if (!w) {
        panic("Failed to create world");
    }
    return w;
}

void notmain() {
    player_t player = {.player_id = 0,
        .position = (pos_t) {0, 0, 0},
        .rotation = (p_rot_t) {0, 0}
    };

    world_t* w = initialize_server();

    // begin pulling from uart to update world
    uart_init();
    trace("Server initialized\n");
    while (1) {
        if (uart_has_data()) {
            char c = (char) uart_get8();
            if (c == 'w' || c == 'a' || c == 's' || c == 'd') {
                do_move(w, c);
            }
            else if (c == 'p' || c == 'r') {
                change_block(c);
            } else if (c == 'm') {
                while (!uart_has_data()) {}
                uint32_t dx = uart_get8();
                while (!uart_has_data()) {}
                uint32_t dy = uart_get8();
                update_rotation(w, dx, dy);
            } else if (c == 'q') {
                uart_flush_tx();
                return;
            }
        }
    }
    uart_flush_tx();
}
