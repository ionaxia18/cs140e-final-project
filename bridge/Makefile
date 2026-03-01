CC = gcc

CFLAGS = -Wall -I../src -I.

BRIDGE_MC_SRC = bridge-minecraft.c
BRIDGE_PI_SRC = bridge-pi.c

all: bridge-minecraft bridge-pi

bridge-minecraft: $(BRIDGE_MC_SRC)
	$(CC) $(CFLAGS) $^ -o bridge-minecraft

bridge-pi: $(BRIDGE_PI_SRC)
	$(CC) $(CFLAGS) $^ -o bridge-pi

clean:
	rm -f bridge-minecraft bridge-pi