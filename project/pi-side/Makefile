PROG_SRC := pi_bridge.c
PROG     := $(PROG_SRC:.c=)
PROG_BIN := $(PROG).bin

MEMMAP  := ../../libpi/memmap
START   := ../../libpi/objs/staff-start.o
ALLOC_SRC  := ../heap/allocator.c
ALLOC_OBJ  := objs/allocator.o
LPI     := ../../libpi/libpi.a
LIBPI   := ../../libpi

ARM := arm-none-eabi
CC  := $(ARM)-gcc

CFLAGS := -O -Wall -Wno-unused-variable -Werror -std=gnu99
CFLAGS += -nostdlib -nostartfiles -ffreestanding
CFLAGS += -mcpu=arm1176jzf-s -mno-unaligned-access
CFLAGS += -I$(LIBPI)/include -I$(LIBPI)/libc -I.. -I../world-state -I../heap

# --- automatically include every .c in ../world-state ---
WORLD_SRCS := $(wildcard ../world-state/*.c)
WORLD_OBJS := $(patsubst ../world-state/%.c,objs/world-state/%.o,$(WORLD_SRCS))

UART_SRCS := uart-helpers.c
UART_OBS  := objs/uart-helpers.o

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

# compile allocator
$(ALLOC_OBJ): $(ALLOC_SRC) | objs
	$(CC) $(CFLAGS) -c $< -o $@

$(UART_OBS): $(UART_SRCS) | objs
	$(CC) $(CFLAGS) -c $< -o $@

# link everything
$(PROG).elf: $(PROG_OBJ) $(WORLD_OBJS) $(ALLOC_OBJ) $(START) $(LPI) $(MEMMAP) $(UART_OBS)
	$(CC) $(CFLAGS) $(START) $(PROG_OBJ) $(WORLD_OBJS) $(ALLOC_OBJ) -T $(MEMMAP) $(LPI) -lgcc -o $@

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