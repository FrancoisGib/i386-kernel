.section .multiboot # multiboot2 header
.L_multiboot_start:
	.equ .L_MULTIBOOT_MAGIC, 0xE85250D6
	.equ .L_MULTIBOOT_ARCH, 0x0 # arch i386
	.equ .L_MULTIBOOT_HEADER_LENGTH, .L_multiboot_end - .L_multiboot_start
	.equ .L_MULTIBOOT_CHECKSUM, -(.L_MULTIBOOT_MAGIC + .L_MULTIBOOT_ARCH + .L_MULTIBOOT_HEADER_LENGTH)
	.equ .L_MULTIBOOT_TAG_TYPE,  0x0
	.equ .L_MULTIBOOT_TAG_FLAGS,  0x0
	.equ .L_MULTIBOOT_TAG_SIZE, 8

    .long .L_MULTIBOOT_MAGIC
    .long .L_MULTIBOOT_ARCH
    .long .L_MULTIBOOT_HEADER_LENGTH
    .long .L_MULTIBOOT_CHECKSUM
    .short .L_MULTIBOOT_TAG_TYPE
    .short .L_MULTIBOOT_TAG_FLAGS
    .long .L_MULTIBOOT_TAG_SIZE
.L_multiboot_end:

.extern _kernel_stack_top
.extern main

.section .text
.global _start
.type _start, @function
_start:
	movl $_kernel_stack_top, %esp
	call main
	cli
	hlt
