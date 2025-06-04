BUILD_NAME = main
IMAGE = $(BUILD_NAME).iso
BIN = $(BUILD_NAME).bin
INC_DIR = include
SRC_DIR = src
BUILD_DIR = build

CC = gcc
LD = ld
AS = nasm
OBJCOPY = objcopy

GRUB_DIR = /usr/lib/grub/i386-pc/

CSOURCES = $(wildcard $(SRC_DIR)/*.c)
ASOURCES = $(wildcard $(SRC_DIR)/*.s) # AT&T syntax
ASMSOURCES += $(wildcard $(SRC_DIR)/*.asm) # intel syntax

COBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CSOURCES))
AOBJS = $(patsubst $(SRC_DIR)/%.s, $(BUILD_DIR)/%.o, $(ASOURCES)) 
ASMOBJS = $(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/%.o, $(ASMSOURCES))

ASFLAGS = -felf
LDFLAGS = -Tlink.ld -melf_i386 -z noexecstack
ARCHFLAGS = -m32

CFLAGS = -I$(INC_DIR) -std=c2x -O3 -g -ffreestanding -nostdlib -fno-pic -fno-pie -no-pie \
	-Wall \
	-Wextra \
	-Werror \
	-Wformat \
	-Wstrict-prototypes \
	-Wunreachable-code \
	-Wcast-align \
	-Wshadow \
	-Wswitch-default \
	-Wswitch-enum

STRIP_SYMBOLS = cursor_x \
		cursor_y
STRIP_FLAGS = $(addprefix --strip-symbol=, $(STRIP_SYMBOLS))


all: $(IMAGE)

run: $(IMAGE)
	qemu-system-i386 -cdrom $(IMAGE)

$(IMAGE): $(BUILD_DIR) | $(BUILD_DIR)/$(BIN)
	mkdir -p $(BUILD_DIR)/boot/grub
	cp grub.cfg $(BUILD_DIR)/boot/grub
	grub-mkrescue -d $(GRUB_DIR) -o $(IMAGE) $(BUILD_DIR)

$(BUILD_DIR)/$(BIN): $(AOBJS) $(ASMOBJS) $(COBJS)
	$(LD) $(LDFLAGS) $^ -o $@
	$(OBJCOPY) $(STRIP_FLAGS) $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(ARCHFLAGS) -c $< -o $@

# AT&T syntax
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	$(CC) $(ARCHFLAGS) -c $< -o $@

# intel syntax
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(IMAGE)
