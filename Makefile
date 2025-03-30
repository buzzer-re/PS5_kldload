
PS5_HOST ?= 192.168.88.11
PS5_PORT ?= 9021

ifdef PS5_PAYLOAD_SDK
    include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk
else
    $(error PS5_PAYLOAD_SDK is undefined)
endif

CFLAGS := -Wall -Werror
KSTUFF_PATH := playstation_research_utils/ps5_kernel_research/kstuff-no-fpkg/ps5-kstuff/

all: payload_bin.c kldload.elf

payload_bin.c: $(KSTUFF_PATH)
	make -C $^
	@cp $^/payload.bin payload.bin
	@xxd -i payload.bin | sed 's/\bunsigned\b/static unsigned/g' > src/$@

kldload.elf:
	@rm -f *.o *.elf
	$(CC) -o $@ src/*.c src/*.asm -O0
	strip $@
	
clean: $(KSTUFF_PATH)
	make clean -C $^
	rm -f *.o *.elf

test: kldload.elf
	$(PS5_DEPLOY) -h $(PS5_HOST) -p $(PS5_PORT) $^

