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
CFLAGS += -I$(LIBPI)/include -I$(LIBPI)/libc -I.. -I../world-state -I../heap -I../gpio

WORLD_SRCS := $(wildcard ../world-state/*.c)
WORLD_OBJS := $(patsubst ../world-state/%.c,objs/world-state/%.o,$(WORLD_SRCS))

UART_SRCS := uart-helpers.c
UART_OBJS := objs/uart-helpers.o

GPIO_SRCS := $(wildcard ../gpio/*.c)
GPIO_OBJS := $(patsubst ../gpio/%.c,objs/gpio/%.o,$(GPIO_SRCS))

PROG_OBJ := objs/$(PROG).o

all: $(PROG_BIN)

objs:
	mkdir -p objs
	mkdir -p objs/world-state
	mkdir -p objs/gpio

objs/%.o: %.c | objs
	$(CC) $(CFLAGS) -c $< -o $@

objs/world-state/%.o: ../world-state/%.c | objs
	$(CC) $(CFLAGS) -c $< -o $@

objs/gpio/%.o: ../gpio/%.c | objs
	$(CC) $(CFLAGS) -c $< -o $@

$(ALLOC_OBJ): $(ALLOC_SRC) | objs
	$(CC) $(CFLAGS) -c $< -o $@

$(UART_OBJS): $(UART_SRCS) | objs
	$(CC) $(CFLAGS) -c $< -o $@

$(PROG).elf: $(PROG_OBJ) $(WORLD_OBJS) $(GPIO_OBJS) $(ALLOC_OBJ) $(UART_OBJS) $(START) $(LPI) $(MEMMAP)
	$(CC) $(CFLAGS) $(START) $(PROG_OBJ) $(WORLD_OBJS) $(GPIO_OBJS) $(ALLOC_OBJ) $(UART_OBJS) -T $(MEMMAP) $(LPI) -lgcc -o $@

$(PROG_BIN): $(PROG).elf
	$(ARM)-objcopy $< -O binary $@
	$(ARM)-objdump -D $< > $(PROG).list

run: $(PROG_BIN)
	pi-install $(PROG_BIN)

clean:
	rm -rf objs
	rm -f *.bin *.elf *.list *~

.PHONY: all clean run