#ifndef BRIDGE_PI_H
#define BRIDGE_PI_H

void disable_raw_mode();
void enable_raw_keyboard_mode() ;
void send_block(int x, int y, int z, char *block);

#endif