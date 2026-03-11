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

# --- Object Mapping ---

# 1. World State
WORLD_SRCS := $(wildcard ../world-state/*.c)
WORLD_OBJS := $(patsubst ../world-state/%.c,objs/world-state/%.o,$(WORLD_SRCS))

# 2. GPIO
GPIO_SRCS  := $(wildcard ../gpio/*.c)
GPIO_OBJS  := $(patsubst ../gpio/%.c,objs/gpio/%.o,$(GPIO_SRCS))

# 3. External Helpers (emmc.c, unicode.c)
EXTERNAL_SRCS := ../filesystems/external-code/unicode-utf8.c ../filesystems/external-code/emmc.c
EXTERNAL_OBJS := $(patsubst ../filesystems/external-code/%.c,objs/external-code/%.o,$(EXTERNAL_SRCS))

# 4. Filesystems (fat32.c, mbr.c, pi-sd.c, boot_server.c)
FS_SRCS    := $(wildcard ../filesystems/*.c)
FS_OBJS    := $(patsubst ../filesystems/%.c,objs/filesystems/%.o,$(FS_SRCS))

# 5. Boot (world-gen.c)
BOOT_SRCS  := $(wildcard ../boot/*.c)
BOOT_OBJS  := $(patsubst ../boot/%.c,objs/boot/%.o,$(BOOT_SRCS))

# 6. Utilities and Staff
STAFF_KMALLOC := $(LIBPI)/staff-objs/staff-kmalloc.o
ALLOC_OBJ  := objs/allocator.o
UART_OBJS  := objs/uart-helpers.o
PROG_OBJ   := objs/$(PROG).o

# --- THE MASTER LIST ---
# Added $(EXTERNAL_OBJS) here! 
ALL_OBJS := $(PROG_OBJ) $(WORLD_OBJS) $(GPIO_OBJS) $(FS_OBJS) $(BOOT_OBJS) $(EXTERNAL_OBJS) $(ALLOC_OBJ) $(UART_OBJS) $(STAFF_KMALLOC)

all: $(PROG_BIN)

# --- Compilation Rules ---

# Local files
objs/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Folder-specific rules
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

# Rule for external-code (fixed path)
objs/external-code/%.o: ../filesystems/external-code/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(ALLOC_OBJ): ../heap/allocator.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# --- Linking ---

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