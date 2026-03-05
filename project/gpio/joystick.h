#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#include "rpi.h"
#include "spi.h"
#include "gpio.h"

#define LED_PIN 26

void joystick_init(void);
void read_joystick(void);

#endif