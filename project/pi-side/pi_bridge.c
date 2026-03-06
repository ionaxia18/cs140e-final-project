// #include "pi_bridge.h"
#include "../../libpi/rpi.h"
#include "../constants.h"
// #include "../world-state/world.h"
#include "../world-state/player.h"
#include "../world-state/world-gen.h"
#include "../world-state/pending.h"
#include "../world-state/hashtable.h"

#define BAUDRATE B115200

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
void do_move(char c, player_t* player) {
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
    uart_put_str("\r\n");
 }

void change_block(char c, world_t* w, player_t* player) {
    // this depends on how we update the player rotation ?
    pos_t block_pos = {0, 0, 0};
    block_t block = pointing_block(w, player, &block_pos);
    if (c == 'p') {
        world_set_block(w, block_pos, block);
    }
    else if (c == 'r') {
        world_set_block(w, block_pos, BLOCK_AIR);
    }
    uart_put_str("BLOCK ");
    uart_put8(' ');
    uart_put_int(block_pos.x);
    uart_put8(' ');
    uart_put_int(block_pos.y);
    uart_put8(' ');
    uart_put_int(block_pos.z);
    uart_put8(' ');
    uart_put_int(block);
    uart_put8('\n');
}

 
void update_rotation(player_t* player, uint16_t yaw, uint16_t pitch) {
    player_rotation_increment(player, yaw, pitch);
    // idk how to scale the rotation
    uart_put_str("ROT ");
    uart_put_int(player->rotation.yaw);
    uart_put_str(" ");
    uart_put_int(player->rotation.pitch);
    uart_put_str("\n");
}

world_t* initialize_server() {
    // initialize world seed
    world_info_t info = {
        .seed = 0,
        .min = (pos_t){0, -60, 0},
        .max = (pos_t){16, -44, 16},
        .edits_cap = 4096,
        .pending_cap = 1024,
    };

    world_t* w = world_create(&info);
    if (!w) {
        panic("Failed to create world");
        return w;
    }
    return w;
}

void notmain() {
    player_t player = {.player_id = 0,
        .position = (pos_t) {0, 0, -60},
        .rotation = (p_rot_t) {0, 0}
    };

    world_t* w = initialize_server();

    // outdated logic, needs to pull from gpio
    // begin pulling from uart to update world
    uart_init();
    trace("Server initialized\n");
    while (1) {
        if (uart_has_data()) {
            char c = (char) uart_get8();
            if (c == 'w' || c == 'a' || c == 's' || c == 'd') {
                do_move(c, &player);
            }
            else if (c == 'p' || c == 'r') {
                change_block(c, w, &player);
            } else if (c == 'm') {
                while (!uart_has_data()) {}
                uint32_t dx = uart_get8();
                while (!uart_has_data()) {}
                uint32_t dy = uart_get8();
                update_rotation(&player, dx, dy);
            } else if (c == 'q') {
                uart_flush_tx();
                return;
            }
        }
    }
    uart_flush_tx();
}
