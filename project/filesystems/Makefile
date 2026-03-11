PROG_SRC := test_fn.c
PROG     := $(PROG_SRC:.c=)
PROG_BIN := $(PROG).bin

ARM := arm-none-eabi
CC  := $(ARM)-gcc

LIBPI   := ../../libpi
LPI     := $(LIBPI)/libpi.a
START   := $(LIBPI)/objs/staff-start.o
MEMMAP  := $(LIBPI)/memmap

STAFF_KMALLOC := $(LIBPI)/staff-objs/staff-kmalloc.o

PROG_OBJ := objs/$(PROG).o
BOOT_OBJ := objs/boot_server.o
ALLOC_OBJ := objs/allocator.o

SD_SRCS := pi-sd.c fat32.c fat32-helpers.c fat32-lfn-helpers.c mbr.c mbr-helpers.c ../boot/world-gen.c ../pi-side/uart-helpers.c
SD_OBJS := $(patsubst %.c,objs/%.o,$(SD_SRCS))

EXTERNAL_SRCS :=  external-code/unicode-utf8.c external-code/emmc.c
EXTERNAL_OBJS := $(patsubst external-code/%.c,objs/external-code/%.o,$(EXTERNAL_SRCS))

WORLD_SRCS := $(wildcard ../world-state/*.c)
WORLD_OBJS := $(patsubst ../world-state/%.c,objs/world-state/%.o,$(WORLD_SRCS))

GPIO_SRCS := $(wildcard ../gpio/*.c)
GPIO_OBJS := $(patsubst ../gpio/%.c,objs/gpio/%.o,$(GPIO_SRCS))

CFLAGS := -O -Wall -Wno-pointer-sign -std=gnu99 
CFLAGS += -nostdlib -nostartfiles -ffreestanding 
CFLAGS += -mcpu=arm1176jzf-s -mno-unaligned-access

INCLUDES := -I. -I.. -I$(LIBPI) -I$(LIBPI)/include -I$(LIBPI)/libc -I../world-state -I../heap -I../gpio -Iexternal-code
CFLAGS += $(INCLUDES)

all: $(PROG_BIN)
objs/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

objs/world-state/%.o: ../world-state/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

objs/gpio/%.o: ../gpio/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

objs/external-code/%.o: external-code/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(ALLOC_OBJ): ../heap/allocator.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(PROG).elf: $(START) $(PROG_OBJ) $(BOOT_OBJ) $(SD_OBJS) $(EXTERNAL_OBJS) $(WORLD_OBJS) $(GPIO_OBJS) $(ALLOC_OBJ) $(STAFF_KMALLOC) $(LPI) $(UART_OBJ)
	$(CC) $(CFLAGS) \
	$(START) \
	$(PROG_OBJ) \
	$(BOOT_OBJ) \
	$(SD_OBJS) \
	$(EXTERNAL_OBJS) \
	$(WORLD_OBJS) \
	$(GPIO_OBJS) \
	$(ALLOC_OBJ) \
	$(STAFF_KMALLOC) \
	$(UART_OBJ) \
	-T $(MEMMAP) \
	$(LPI) \
	-lgcc \
	-o $@

$(PROG_BIN): $(PROG).elf
	$(ARM)-objcopy $< -O binary $@
	$(ARM)-objdump -D $< > $(PROG).list

clean:
	rm -rf objs *.bin *.elf *.list *~

.PHONY: all clean