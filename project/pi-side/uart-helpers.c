#include "../../libpi/rpi.h"
#include "uart-helpers.h"

// Serializes a signed integer to ASCII and writes it byte-by-byte on UART.
// This avoids stdlib formatting and keeps protocol output deterministic.
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

// Serializes a float as "<int>[.<3 digits>]" and writes it on UART.
// Current protocol precision is fixed at 3 decimal places.
void uart_put_float(float f) {
    if (f < 0) {
        uart_put8('-');
        f = -f;
    }

    int int_part = (int)f;
    uart_put_int(int_part);

    float frac = f - int_part;
    if (frac == 0) {
        return;
    }

    uart_put8('.');
    for (int i = 0; i < 3; i++) {   // 3 decimal places
        frac *= 10;
        int digit = (int)frac;
        uart_put8('0' + digit);
        frac -= digit;
    }
}
// Writes a null-terminated C string directly to UART.
void uart_put_str(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        uart_put8(str[i]);
    }
}

// Emits one BLOCK protocol line:
//   BLOCK <x> <y-1> <z> <block_id>\n
// The y-1 transform keeps host-side coordinates aligned with FruitJuice's
// interpretation of player/world height.
void send_set_block(pos_t p, block_t new_block) {
    uart_put_str("BLOCK ");
    uart_put_float(p.x);
    uart_put8(' ');
    // position stored in world one above how fruit juice stores it ?
    uart_put_float(p.y - 1.0f);
    uart_put8(' ');
    uart_put_float(p.z);
    uart_put8(' ');
    uart_put_int(new_block);
    uart_put8('\n');
}

// Emits one PLAYER protocol line:
//   PLAYER <x> <y-1> <z>\n
// The y-1 transform matches the same convention used by send_set_block().
void send_player_move(player_t* p) {
    uart_put_str("PLAYER ");
    uart_put_float(p->position.x);
    uart_put_str(" ");
    // position stored in world one above how fruit juice stores it ?
    uart_put_float(p->position.y - 1.0f);
    uart_put_str(" ");
    uart_put_float(p->position.z);
    uart_put_str("\n");
}

// Emits one ROT protocol line:
//   ROT <yaw> <pitch>\n
void send_player_rotation(player_t* p) {
    uart_put_str("ROT ");
    uart_put_int(p->rotation.yaw);
    uart_put_str(" ");
    uart_put_int(p->rotation.pitch);
    uart_put_str("\n");
}

// Replays every edited block currently stored in world->edits as BLOCK lines.
// Intended for host resync after reconnect/load, not for per-frame updates.
void send_world(world_t *w) {
    trace("sending world now\n");
    for (uint32_t i = 0; i < w->edits.cap; i++) {
        // trace("i = %d", i);
        world_entry_t* cur = w->edits.entries[i];
        while (cur) {
            world_entry_t* next = cur->next;
            send_set_block(cur->pos, cur->block);
            cur = next;
        }
    }
    trace("edit size=%d\n", w->edits.size);
}