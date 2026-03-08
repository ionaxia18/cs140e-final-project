#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#include "rpi.h"
#include "spi.h"
#include "gpio.h"
#include "../world-state/player.h"

// init gpio output pin for joystick led
void joystick_init(void);

// read the joystick and print x, y, whether the button is pressed
// turns on led if button is pressed
int read_joystick(p_rot_t*);
#endif