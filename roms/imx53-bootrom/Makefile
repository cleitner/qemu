CROSS_COMPILE := arm-linux-gnueabihf-

CC := $(CROSS_COMPILE)gcc
CFLAGS := -marm -march=armv7-a -mcpu=cortex-a8 -fpic -g -O0
LDFLAGS := -nostartfiles -nodefaultlibs -nostdlib -Wl,-Ttext=0x00000000 -Wl,--section-start=.rodata=0x0000F000

imx53-bootrom.elf: bootrom.o print.o
	$(CC) $(LDFLAGS) -o $@ $^

