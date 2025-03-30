
PS5_HOST ?= ?
PS5_PORT ?= 9021

ifdef PS5_PAYLOAD_SDK
    include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk
else
    $(error PS5_PAYLOAD_SDK is undefined)
endif

CFLAGS := -Wall -Werror

all: kekcall.elf

kekcall.elf:
	$(CC) -o $@ *.c *.asm -O0
	strip $@
	
clean:
	rm -f payload_bin.c *.o *.elf

test: kekcall.elf
	$(PS5_DEPLOY) -h $(PS5_HOST) -p $(PS5_PORT) $^

