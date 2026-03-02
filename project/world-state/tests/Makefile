PROG_SRC := $(wildcard *.c)
PROGS := $(PROG_SRC:.c=.bin)

LIBPI     := ../../../libpi
WORLD_SRC := ../flatworld.c

MEMMAP := $(LIBPI)/memmap
START  := $(LIBPI)/staff-start.o
KMALLOC := $(LIBPI)/staff-objs/staff-kmalloc.o
LPI    := $(LIBPI)/libpi.a

ARM  := arm-none-eabi
CC   := $(ARM)-gcc

CFLAGS := -O -Wall -Wno-unused-variable -Werror -std=gnu99
CFLAGS += -nostdlib -nostartfiles -ffreestanding
CFLAGS += -mcpu=arm1176jzf-s
CFLAGS += -mno-unaligned-access
CFLAGS += -I$(LIBPI)/include -I$(LIBPI)/libc -I..

all: $(PROGS) run

%.bin: %.c $(WORLD_SRC) $(START) $(LPI) $(MEMMAP) Makefile
	$(CC) $(CFLAGS) $(START) $< $(WORLD_SRC) $(KMALLOC) -T $(MEMMAP) $(LPI) -o $*.elf
	$(ARM)-objcopy $*.elf -O binary $@
	$(ARM)-objdump -D $*.elf > $*.list

run: $(PROGS)
	@for bin in $(PROGS); do \
		echo "--- Running $$bin ---"; \
		pi-install $$bin; \
	done

clean:
	rm -f *.bin *.list *.elf *.o *~

.PHONY: all clean run
