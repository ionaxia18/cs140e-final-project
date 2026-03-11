# NOTE: "make check" is extremely weak for today --- 
# we don't know what is on your microsd so can't check
# in general.  its mainly useful so you can run
# a bunch of tests automatically and get notified if
# something failed.
#
# when developing: you should probably run "make" 
# and look at the output to make sure it makes sense.

# swap these when you build <mbr.c>
COMMON_SRC += ../../../fat32/mbr.c

# swap these when you build <fat32.c>
COMMON_SRC += ../../../fat32/fat32.c

PROGS := test-flat-mtn.c


CFLAGS_EXTRA  = -I../../../fat32/external-code

# a list of all of your object files.
# pi-sd.c #includes external-code/emmc.c — do not add emmc.c or you get multiple definition.
# fat32-lfn-helpers.c only includes unicode-utf8.h, so we must link unicode-utf8.c.
COMMON_SRC += ../../../fat32/pi-sd.c ../../../fat32/mbr-helpers.c ../../../fat32/fat32-helpers.c ../../../fat32/fat32-lfn-helpers.c ../../../fat32/external-code/unicode-utf8.c ../../pi-side/uart-helpers.c
COMMON_SRC += ../world-gen.c 
O := $(CS140E_2026_PATH)/libpi
STAFF_OBJS += $(O)/staff-objs/staff-kmalloc.o
# to make sure that it handles GPIO pin 54
STAFF_OBJS += $(O)/staff-objs/gpio.o

# external-code/bzt-sd.c 

TTYUSB = 
BOOTLOADER = my-install
RUN = 1

EXCLUDE ?= grep -v simple_boot
GREP_STR := 'HASH:\|ERROR:\|PANIC:\|PASS:\|TEST:'
include $(CS140E_2026_PATH)/libpi/mk/Makefile.robust-v3

checkoff:
	make -C ./ "PROGS=$(ALL_PROGS)"

clean::
	rm -f tests/*.list tests/*.o tests/*.elf tests/*~ tests/*.bin
	make -C ../../../fat32/external-code clean
