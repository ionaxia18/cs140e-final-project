PROG_SRC := pi_bridge.c
PROG_BIN := $(PROG_SRC:.c=.bin)

MEMMAP    := ../../libpi/memmap
START     := ../../libpi/objs/staff-start.o
KMALLOC   := ../../libpi/staff-objs/staff-kmalloc.o
LPI       := ../../libpi/libpi.a
UART	  := ../../libpi/objs/uart.o

ARM := arm-none-eabi
CC  := $(ARM)-gcc

CFLAGS := -O -Wall -Wno-unused-variable -Werror -std=gnu99
CFLAGS += -nostdlib -nostartfiles -ffreestanding
CFLAGS += -mcpu=arm1176jzf-s
CFLAGS += -mno-unaligned-access
CFLAGS += -I$(LIBPI)/include -I$(LIBPI)/libc -I..

all: $(PROG_BIN)

$(PROG_BIN): $(PROG_SRC) $(START) $(LPI) $(UART) $(MEMMAP)
	$(CC) $(CFLAGS) $(START) $(PROG_SRC) $(KMALLOC) -T $(MEMMAP) $(LPI) -o $*.elf
	$(ARM)-objcopy $*.elf -O binary $@
	$(ARM)-objdump -D $*.elf > $*.list

run: $(PROG_BIN)
	pi-install $(PROG_BIN)

clean:
	rm -f *.bin *.elf *.o *.list *~

.PHONY: all clean run