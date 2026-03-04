PROG_SRC := pi_bridge.c
PROG     := $(PROG_SRC:.c=)
PROG_BIN := $(PROG).bin

MEMMAP  := ../../libpi/memmap
START   := ../../libpi/objs/staff-start.o
KMALLOC := ../../libpi/staff-objs/staff-kmalloc.o
LPI     := ../../libpi/libpi.a
LIBPI   := ../../libpi

ARM := arm-none-eabi
CC  := $(ARM)-gcc

CFLAGS := -O -Wall -Wno-unused-variable -Werror -std=gnu99
CFLAGS += -nostdlib -nostartfiles -ffreestanding
CFLAGS += -mcpu=arm1176jzf-s -mno-unaligned-access
CFLAGS += -I$(LIBPI)/include -I$(LIBPI)/libc -I.. -I../world-state

# --- automatically include every .c in ../world-state ---
WORLD_SRCS := $(wildcard ../world-state/*.c)
WORLD_OBJS := $(patsubst ../world-state/%.c,objs/world-state/%.o,$(WORLD_SRCS))

# main program object
PROG_OBJ := objs/$(PROG).o

all: $(PROG_BIN)

# build output directories if missing
objs:
	mkdir -p objs
	mkdir -p objs/world-state

# compile main
objs/%.o: %.c | objs
	$(CC) $(CFLAGS) -c $< -o $@

# compile world-state files
objs/world-state/%.o: ../world-state/%.c | objs
	$(CC) $(CFLAGS) -c $< -o $@

# link everything
$(PROG).elf: $(PROG_OBJ) $(WORLD_OBJS) $(START) $(KMALLOC) $(LPI) $(MEMMAP)
	$(CC) $(CFLAGS) $(START) $(PROG_OBJ) $(WORLD_OBJS) $(KMALLOC) -T $(MEMMAP) $(LPI) -o $@

# convert to .bin and dump
$(PROG_BIN): $(PROG).elf
	$(ARM)-objcopy $< -O binary $@
	$(ARM)-objdump -D $< > $(PROG).list

run: $(PROG_BIN)
	pi-install $(PROG_BIN)

clean:
	rm -rf objs
	rm -f *.bin *.elf *.list *~

.PHONY: all clean run