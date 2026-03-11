
PS5_HOST ?= ps5
PS5_PORT ?= 9021

ifdef PS5_PAYLOAD_SDK
    include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk
else
    $(error PS5_PAYLOAD_SDK is undefined)
endif

CFLAGS := -Wall -Werror

ifndef LLVM_CONFIG
    LLVM_CONFIG := $(PS5_PAYLOAD_SDK)/bin/llvm-config
    export LLVM_CONFIG
endif

all: kldload.elf

kldload.elf:
	@rm -f *.o *.elf
	$(CC) -o $@ src/*.c -O0
	strip $@

clean:
	rm -f *.o *.elf

test: kldload.elf
	$(PS5_DEPLOY) -h $(PS5_HOST) -p $(PS5_PORT) $^
