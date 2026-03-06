#ifndef __MATRIX_H__
#define __MATRIX_H__

#include "rpi.h"
#include "gpio.h"
#include "../world-state/world.h"

#define MATRIX_SIZE 4
#define COL_BASE 16
#define ROW_BASE 20

// init gpio input and output pins for matrix
void matrix_init(void);

// read the block pressed on the matrix
// returns the block number 1-16 or 0 if no block is pressed
block_t read_block(void);

#endif