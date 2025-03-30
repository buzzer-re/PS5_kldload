
CC		:= gcc
OBJCOPY	:= objcopy
ODIR	:= build
SDIR	:= src

MAPFILE := $(shell basename "$(CURDIR)").map
CFLAGS	:= $(IDIRS) -Os -std=c11 -ffunction-sections -fdata-sections -fno-builtin -nostartfiles -nostdlib -Wall -Wextra -march=btver2 -mtune=btver2 -m64 -mabi=sysv -mcmodel=small -fpie -fPIC
CFLAGS += -DFIRMWARE=$(FIRMWARE)

LFLAGS	:= -Xlinker -T ./linker.x -Xlinker -Map="$(MAPFILE)" -Wl,--build-id=none -Wl,--gc-sections
CFILES	:= $(wildcard $(SDIR)/*.c)
SFILES	:= $(wildcard $(SDIR)/*.s)
OBJS	:= $(patsubst $(SDIR)/%.c, $(ODIR)/%.o, $(CFILES)) $(patsubst $(SDIR)/%.s, $(ODIR)/%.o, $(SFILES)) $(CFILES)

TARGET = $(shell basename "$(CURDIR)").bin
ELF    = $(shell basename "$(CURDIR)").elf

$(TARGET): $(ODIR) $(OBJS)
	$(CC) $(ODIR)/*.o -o $(ELF) $(CFLAGS) $(LFLAGS) $(LIBS)
	$(OBJCOPY) -S -O binary $(ELF) "$(TARGET)"
	mv $(ELF) build/

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(SDIR)/%.s
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR):
	@mkdir -p build/

.PHONY: clean

clean:
	rm -rf "$(TARGET)" "$(MAPFILE)" $(ODIR)