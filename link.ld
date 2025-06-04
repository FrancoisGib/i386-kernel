ENTRY(_start)
OUTPUT_FORMAT("elf32-i386")

base_address = 0x100000;

SECTIONS
{
	. = base_address;

	.text :
	{
		*(.multiboot)
		*(.text*)
		. = ALIGN(4096);
	}

	.rodata :
	{
		*(.rodata*)
		. = ALIGN(4096);
	}

	.data :
	{
		*(.data*)
		. = ALIGN(4096);
	}

	.bss :
	{
		*(.bss*)
		*(COMMON)
		. = ALIGN(4096);
	}

	.stack :
	{
		_stack_bot = .; 
		. += 16384;
		_stack_top = .;
		. = ALIGN(4096);
	}
}