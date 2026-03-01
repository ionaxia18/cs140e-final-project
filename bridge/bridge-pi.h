#ifndef BRIDGE_PI_H
#define BRIDGE_PI_H

void bridge_pi_init(void);
int read_line(char *buf, int max_size);
void send_string(const char * s);
void send_block(int x, int y, int z, char *block);

#endif