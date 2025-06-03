BUILD_NAME = main
IMAGE = $(BUILD_NAME).iso
BIN = $(BUILD_DIR)/$(BUILD_NAME).bin
INC_DIR = include
SRC_DIR = src
BUILD_DIR = build

CC = gcc
LD = ld
AS = as

GRUB_DIR = /usr/lib/grub/i386-pc/

CSOURCES = $(wildcard $(SRC_DIR)/*.c)
ASOURCES = $(wildcard $(SRC_DIR)/*.s)

COBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CSOURCES))
AOBJS = $(patsubst $(SRC_DIR)/%.s, $(BUILD_DIR)/%.o, $(ASOURCES))

ARCHFLAGS =-m32

ASFLAGS =
LDFLAGS = -melf_i386 -Tlink.ld

CFLAGS = -std=c99 -O3 -g -ffreestanding -nostdlib \
	-Wall \
	-Wextra \
	-Werror \
	-Wformat \
	-Wstrict-prototypes \
	-Wunreachable-code \
	-Wcast-align \
	-Wshadow \
	-pedantic \
	-Wswitch-default \
	-Wswitch-enum

all: $(IMAGE)

$(IMAGE): $(BUILD_DIR) | $(BIN)
	mkdir -p $(BUILD_DIR)/boot/grub
	cp grub.cfg $(BUILD_DIR)/boot/grub
	grub-mkrescue -d $(GRUB_DIR) -o $(IMAGE) $(BUILD_DIR)

$(BIN): $(AOBJS) $(COBJS)
	$(LD) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(ARCHFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	$(CC) $(ASFLAGS) $(ARCHFLAGS) -c $< -o $@

qemu: $(BUILD_DIR) | $(ELFFILE)
	qemu-system-i386 -cdrom $(IMAGE)

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(IMAGE)
