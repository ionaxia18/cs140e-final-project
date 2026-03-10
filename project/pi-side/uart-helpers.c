#include "../../libpi/rpi.h"
#include "uart-helpers.h"
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
void uart_put_str(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        uart_put8(str[i]);
    }
}

void send_set_block(pos_t p, block_t new_block) {
    uart_put_str("BLOCK ");
    uart_put8(' ');
    uart_put_float(p.x);
    uart_put8(' ');
    uart_put_float(p.y);
    uart_put8(' ');
    uart_put_float(p.z);
    uart_put8(' ');
    uart_put_int(new_block);
    uart_put8('\n');
}

// assumes position is valid, id not being used, unsure how fruitjuice likes it
void send_player_move(player_t* p) {
    uart_put_str("PLAYER ");
    uart_put_float(p->position.x);
    uart_put_str(" ");
    uart_put_float(p->position.y);
    uart_put_str(" ");
    uart_put_float(p->position.z);
    uart_put_str("\n");
}

void send_player_rotation(player_t* p) {
    uart_put_str("ROT ");
    uart_put_int(p->rotation.yaw);
    uart_put_str(" ");
    uart_put_int(p->rotation.pitch);
    uart_put_str("\n");
}