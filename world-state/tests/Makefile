PROG_SRC := test-key-fns.c

# 2) For each PROG_SRC X.c we build a X.bin
PROGS := $(PROG_SRC:.c=.bin)

LIBPI := ../../libpi
WORLD_SRC := ../world.c

MEMMAP := $(LIBPI)/memmap
START := $(LIBPI)/staff-start.o
LPI   := $(LIBPI)/libpi.a

ARM   := arm-none-eabi
CC    := $(ARM)-gcc

CFLAGS := -O -Wall -Wno-unused-variable -Werror -std=gnu99
CFLAGS += -nostdlib -nostartfiles -ffreestanding
CFLAGS += -mcpu=arm1176jzf-s
CFLAGS += -mno-unaligned-access
CFLAGS += -I$(LIBPI)/include -I..

# 5) The default target: build all binaries.
all: $(PROGS)

%.bin: %.c $(WORLD_SRC) $(START) $(LPI) $(MEMMAP) Makefile
	$(CC) $(CFLAGS) $(START) $< $(WORLD_SRC) -T $(MEMMAP) $(LPI) -o $*.elf
	$(ARM)-objcopy $*.elf -O binary $@
	$(ARM)-objdump -D $*.elf > $*.list

clean:
	rm -f *.bin *.list *.elf *.o *~

.PHONY: all clean
