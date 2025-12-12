PREFIX          = arm-none-eabi-
CC              = $(PREFIX)gcc
AR              = $(PREFIX)ar

CFLAGS          = -Wall
CFLAGS         += -Wextra
CFLAGS         += -Werror
CFLAGS         += -mthumb
CFLAGS         += -Isrc/include
CFLAGS         += -mcpu=cortex-m4
CFLAGS         += -mfloat-abi=hard
CFLAGS         += -mfpu=fpv4-sp-d16
CFLAGS         += -msingle-pic-base
CFLAGS         += -mpic-register=sl
CFLAGS         += -mno-pic-data-is-text-relative
CFLAGS         += -fPIC
CFLAGS         += -ffreestanding
ifdef DEBUG
CFLAGS         += -Og
CFLAGS         += -ggdb
else
CFLAGS         += -Os
endif
CFLAGS         += -Wno-unused-parameter

all: build/libfae.a build/crt0.fae

build :
	mkdir -p build

build/libfae.a: build/stdriot.o
	$(AR) rcs build/libfae.a build/stdriot.o

build/stdriot.o: src/stdriot.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/crt0.fae:
	make -C crt0 all DEBUG=$(DEBUG)

build/crt0.elf:
	make -C crt0 all DEBUG=$(DEBUG)

clean:
	$(RM) build/stdriot.o
	make -C crt0 clean

realclean: clean
	make -C crt0 realclean
	$(RM) -rf build

.PHONY: all clean realclean
