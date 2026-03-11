// #include "pi_bridge.h"
// #include "pi_bridge.h"
#include "../../libpi/rpi.h"
#include "../constants.h"
// #include "../world-state/world.h"
#include "../world-state/player.h"
#include "../world-state/world-gen.h"
#include "../world-state/pending.h"
#include "../world-state/hashtable.h"

#include "../gpio/arcade.h"
#include "../gpio/joystick.h"
#include "../gpio/matrix.h"

#include "uart-helpers.h"
#define BAUDRATE B115200

// takes in a character move, will move the player on pi side and return new coordinates

void do_move(player_t* player, pos_t new_pos) {
    if (!world_pos_is_valid(new_pos)) {
        panic("invalid move to position %d %d %d", new_pos.x, new_pos.y, new_pos.z);
    } 
    player->position = new_pos;
    send_player_move(player);
 }

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

    if (!world_set_block(w, place, block_selected)) {
        trace("world_set_block FAILED at (%d,%d,%d)\n",
                (int)place.x, (int)place.y, (int)place.z);
    }
    send_set_block(place, block_selected);
}
 
// void update_rotation(player_t* player, uint16_t yaw, uint16_t pitch) {
//     player_rotation_increment(player, yaw, pitch);
    // idk how to scale the rotation
    // uart_put_str("ROT ");
    // uart_put_int(player->rotation.yaw);
    // uart_put_str(" ");
    // uart_put_int(player->rotation.pitch);
    // uart_put_str("\n");
// }

world_t* initialize_server() {
    // initialize world seed
    world_info_t info = {
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
        return w;
    }
    return w;
}

bool position_changed(pos_t cur, pos_t old) {
    return (cur.x != old.x) || (cur.y != old.y) || (cur.z != old.z);
}

bool rotation_changed(p_rot_t cur, p_rot_t old) {
    return (cur.pitch != old.pitch) || (cur.yaw != old.yaw);
}

void notmain() {
    player_t player = {.player_id = 0,
        .position = (pos_t) {0, -59, 0},
        .rotation = (p_rot_t) {0, 0}
    };

    world_t* w = initialize_server();
    matrix_init();
    arcade_init();
    joystick_init();
    uart_init();
    // outdated logic, needs to pull from gpio
    // begin pulling from uart to update world
    pos_t new_pos = player.position;
    p_rot_t last_rot = player.rotation;
    block_t block_selected = 0;
    block_t last_block = 0;  /* for debounce: only place on button press, not hold */
    trace("Server initialized\n");
    send_player_move(&player);
    send_player_rotation(&player);
    uart_flush_tx();
    world_print(w);
    while (1) {
        // pos_t new_pos = arcade_read(&player.position);
        pos_t displacement = arcade_read(&player.position);
        int place = read_joystick(&player.rotation);
        if (displacement.x || displacement.y || displacement.z) {
            new_pos = (pos_t){player.position.x + displacement.x, player.position.y + displacement.y, player.position.z + displacement.z};
            if (valid_player_move(w, &player, new_pos)) {
                player.position = new_pos;
                send_player_move(&player);

            }
        }

        if (rotation_changed(player.rotation, last_rot)) {
            send_player_rotation(&player);
            last_rot = player.rotation;
        }
        if (place) {
            block_selected = BLOCK_AIR;
            change_block(w, &player, block_selected);
        }
        block_selected = read_block();
        /* Only place on press (rising edge), not while held */
        if (block_selected && block_selected != 16 && !last_block) {
            // updates w as well
            change_block(w, &player, block_selected);
        }
        if (block_selected == 16 && !last_block) {
            world_destroy(w);
            return;
        }
        last_block = block_selected;
        // this is placing with joystick, replace with placing with matrix
        // if (!place) {
        //     uart_put_str("   ");
        //     change_block('p', w, &player);
        // }
        delay_ms(50);
        uart_flush_tx();
    }
}
