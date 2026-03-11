PROG_SRC := pi_bridge.c
PROG     := $(PROG_SRC:.c=)
PROG_BIN := $(PROG).bin

MEMMAP  := ../../libpi/memmap
START   := ../../libpi/objs/staff-start.o
LPI     := ../../libpi/libpi.a
LIBPI   := ../../libpi

ARM := arm-none-eabi
CC  := $(ARM)-gcc

CFLAGS := -O -Wall -Wno-unused-variable -std=gnu99
CFLAGS += -nostdlib -nostartfiles -ffreestanding
CFLAGS += -mcpu=arm1176jzf-s -mno-unaligned-access
CFLAGS += -I$(LIBPI)/include -I$(LIBPI)/libc -I.. -I../world-state -I../heap -I../gpio -I../filesystems -I../boot

WORLD_SRCS := $(wildcard ../world-state/*.c)
WORLD_OBJS := $(patsubst ../world-state/%.c,objs/world-state/%.o,$(WORLD_SRCS))

GPIO_SRCS  := $(wildcard ../gpio/*.c)
GPIO_OBJS  := $(patsubst ../gpio/%.c,objs/gpio/%.o,$(GPIO_SRCS))

EXTERNAL_SRCS := ../filesystems/external-code/unicode-utf8.c ../filesystems/external-code/emmc.c
EXTERNAL_OBJS := $(patsubst ../filesystems/external-code/%.c,objs/external-code/%.o,$(EXTERNAL_SRCS))

FS_SRCS    := $(wildcard ../filesystems/*.c)
FS_OBJS    := $(patsubst ../filesystems/%.c,objs/filesystems/%.o,$(FS_SRCS))

BOOT_SRCS  := $(wildcard ../boot/*.c)
BOOT_OBJS  := $(patsubst ../boot/%.c,objs/boot/%.o,$(BOOT_SRCS))

STAFF_KMALLOC := $(LIBPI)/staff-objs/staff-kmalloc.o
ALLOC_OBJ  := objs/allocator.o
UART_OBJS  := objs/uart-helpers.o
PROG_OBJ   := objs/$(PROG).o

ALL_OBJS := $(PROG_OBJ) $(WORLD_OBJS) $(GPIO_OBJS) $(FS_OBJS) $(BOOT_OBJS) $(EXTERNAL_OBJS) $(ALLOC_OBJ) $(UART_OBJS) $(STAFF_KMALLOC)

all: $(PROG_BIN)

objs/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

objs/world-state/%.o: ../world-state/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

objs/gpio/%.o: ../gpio/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

objs/boot/%.o: ../boot/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

objs/filesystems/%.o: ../filesystems/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

objs/external-code/%.o: ../filesystems/external-code/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(ALLOC_OBJ): ../heap/allocator.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


$(PROG).elf: $(ALL_OBJS) $(START) $(LPI) $(MEMMAP)
	$(CC) $(CFLAGS) $(START) $(ALL_OBJS) -T $(MEMMAP) $(LPI) -lgcc -o $@

$(PROG_BIN): $(PROG).elf
	$(ARM)-objcopy $< -O binary $@
	$(ARM)-objdump -D $< > $(PROG).list

run: $(PROG_BIN)
	pi-install $(PROG_BIN)

clean:
	rm -rf objs
	rm -f *.bin *.elf *.list *~

.PHONY: all clean run