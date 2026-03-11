#ifndef UART_HELPERS_H
#define UART_HELPERS_H

#include "../../libpi/rpi.h"
#include "../world-state/player.h"
void uart_put_int(int val);

void uart_put_float(float f);
void uart_put_str(char* str);

void send_set_block(pos_t p, block_t new_block);

void send_player_move(player_t* p);

void send_player_rotation(player_t* p);

#endif // UART_HELPERS_H

