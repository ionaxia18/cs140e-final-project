# PROG_SRC := $(wildcard *.c)	
PROG_SRC += matrix-test.c
# PROG_SRC += test-joystixck.c
# PROG_SRC += arcade-test.c
# PROG_SRC += test-joystick.c

DIRS := ../tests

PROGS := $(PROG_SRC:.c=.bin)

LIBPI     := ../../../libpi
# matrix-test only needs matrix.c; joystick/arcade need player.c + -lgcc
MATRIX_SRC := ../matrix.c
FULL_SRC   := ../joystick.c ../matrix.c ../arcade.c ../../world-state/player.c ../../world-state/hashtable.c ../../world-state/pending.c ../../world-state/world.c ../../world-state/world-gen.c ../../heap/allocator.c ../../pi-side/uart-helpers.c

MEMMAP := $(LIBPI)/memmap
START  := $(LIBPI)/staff-start.o
LPI    := $(LIBPI)/libpi.a

ARM  := arm-none-eabi
CC   := $(ARM)-gcc

CFLAGS := -O -Wall -Wno-unused-variable -Werror -std=gnu99
CFLAGS += -nostdlib -nostartfiles -ffreestanding
CFLAGS += -mcpu=arm1176jzf-s
CFLAGS += -mno-unaligned-access
CFLAGS += -I$(LIBPI)/include -I$(LIBPI)/libc -I.. -I../.. -I../../world-state -I../../heap -I../../pi-side

all: $(PROGS) run

matrix-test.bin: matrix-test.c $(MATRIX_SRC) $(START) $(LPI) $(MEMMAP) Makefile
	$(CC) $(CFLAGS) $(START) $< $(MATRIX_SRC) $(KMALLOC) -T $(MEMMAP) $(LPI) -o matrix-test.elf
	$(ARM)-objcopy matrix-test.elf -O binary $@
	$(ARM)-objdump -D matrix-test.elf > matrix-test.list

%.bin: %.c $(FULL_SRC) $(START) $(LPI) $(MEMMAP) Makefile
	$(CC) $(CFLAGS) $(START) $< $(FULL_SRC) $(KMALLOC) -T $(MEMMAP) $(LPI) -lgcc -o $*.elf
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
