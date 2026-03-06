PROG_SRC := test-uart.c
PROG     := $(PROG_SRC:.c=)
PROG_BIN := $(PROG).bin

LIBPI   := ../../../libpi
MEMMAP  := $(LIBPI)/memmap
START   := $(LIBPI)/objs/staff-start.o
LPI     := $(LIBPI)/libpi.a

ARM := arm-none-eabi
CC  := $(ARM)-gcc

CFLAGS := -O -Wall -Wno-unused-variable -Werror -std=gnu99
CFLAGS += -nostdlib -nostartfiles -ffreestanding
CFLAGS += -mcpu=arm1176jzf-s -mno-unaligned-access
CFLAGS += -I$(LIBPI)/include -I$(LIBPI)/libc

all: $(PROG_BIN)

$(PROG).elf: $(PROG_SRC) $(START) $(LPI) $(MEMMAP)
	$(CC) $(CFLAGS) $(START) $(PROG_SRC) -T $(MEMMAP) $(LPI) -lgcc -o $@

$(PROG_BIN): $(PROG).elf
	$(ARM)-objcopy $< -O binary $@
	$(ARM)-objdump -D $< > $(PROG).list

run: $(PROG_BIN)
	pi-install $(PROG_BIN)

clean:
	rm -f *.bin *.list *.elf *.o *~

.PHONY: all clean run
