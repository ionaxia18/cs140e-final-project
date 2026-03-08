#ifndef __ARCADE_H__
#define __ARCADE_H__

#include "rpi.h"
#include "gpio.h"
#include "joystick.h"
#include "../world-state/world.h"
#include "../world-state/player.h"

#define PIN_RIGHT 24
#define PIN_LEFT 26
#define PIN_UP 20
#define PIN_DOWN 22

void arcade_init(void);

void arcade_read(pos_t*);

#endif