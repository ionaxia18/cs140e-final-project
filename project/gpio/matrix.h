#ifndef __MATRIX_H__
#define __MATRIX_H__

#include "rpi.h"
#include "gpio.h"
#include "../world-state/world.h"

#define MATRIX_SIZE 4
#define COL_BASE 14
#define ROW_BASE 21

// change based on how you wire the things, using labels on the board
// cant wire to 7-11, or 14-15, basically have to use 16-19 + 21,23,25,27
static const int col_pins[4] = {19, 18, 17, 16};   // c1, c2, c3, c4
static const int row_pins[4] = {21, 23, 25, 27};   // r1, r2, r3, r4
// init gpio input and output pins for matrix
void matrix_init(void);

// read the block pressed on the matrix
// returns the block number 1-16 or 0 if no block is pressed
block_t read_block(void);

#endif