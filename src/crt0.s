.section .multiboot
.align 4
.long 0x1BADB002            # magic number
.long 0x00                  # flags
.long -(0x1BADB002 + 0x00)  # checksum

.section .note.GNU-stack,"",@progbits

.extern _stack_top
.extern main

.section .text
.global _start
.type _start, @function
_start:
	movl $_stack_top, %esp
	call main
	cli
	hlt
