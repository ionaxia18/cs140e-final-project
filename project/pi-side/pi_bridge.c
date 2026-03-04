#include "pi_bridge.h"
#include "../../libpi/rpi.h"
#include "../constants.h"
#include "../world-state/world.h"
#include "../world-state/world-gen.h"
#include "../world-state/pending.h"
#include "../world-state/hashtable.h"

#define BAUDRATE B115200

struct coords p_coords = {235, 65, -1};
char * block = "STONE";

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
void do_move(world_t* w, char c) {
    world_pos_t new_coord = w->player_pos;
    if (c == 'w') {
        new_coord.x += 1;
    } else if (c == 'a') {
        new_coord.y -= 1;
    } else if (c == 's') {
        new_coord.x += 1;
    } else if (c == 'd') {
        new_coord.y -= 1;
    }

    if (!world_pos_is_valid(new_coord)) {
        panic("invalid move to position %d %d %d", new_coord.x, new_coord.y, new_coord.z);
    } 
    
    w->player_pos = new_coord;
    
    uart_put_str("PLAYER ");
    uart_put_int(new_coord.x);
    uart_put_str(" ");
    uart_put_int(new_coord.y);
    uart_put_str(" ");
    uart_put_int(new_coord.z);
    uart_put_str("\n");
 }

void change_block(char c) {
    char * cur_block = "";
    if (c == 'p') {
        cur_block = block;
    }
    else if (c == 'r') {
        cur_block = "AIR";
    }
    const char *str = "BLOCK";
    for (size_t i = 0; str[i] != '\0'; i++) {
        uart_put8((uint8_t) str[i]);
    }
    uart_put8(' ');
    uart_put_int(p_coords.x);
    uart_put_int(p_coords.y);
    uart_put_int(p_coords.z);
    for (size_t i = 0; cur_block[i] != '\0'; i++) {
        uart_put8((uint8_t) cur_block[i]);
    }
    uart_put8('\n');
}



void notmain() {
    // initialize world seed
    world_info_t info = {
        .seed = 1,
        .min = (world_pos_t){0, -60, 0},
        .max = (world_pos_t){16, -44, 16},
        .edits_cap = 4096,
        .pending_cap = 4096,
    };

    world_t* w = world_create(&info);


    if (!w) {
        panic("Failed to create world");
    }

    // begin pulling from uart to update world
    uart_init();
    trace("Comparison test passed\n");
    while (1) {
        if (uart_has_data()) {
            char c = (char) uart_get8();
            if (c == 'w' || c == 'a' || c == 's' || c == 'd') {
                do_move(w, c);
            }
            else if (c == 'p' || c == 'r') {
                change_block(c);
            } 
            else if (c == 'q') {
                uart_flush_tx();
                return;
            }
        }
    }
    uart_flush_tx();
}
