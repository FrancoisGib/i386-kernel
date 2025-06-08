ENTRY(_start)
OUTPUT_FORMAT("elf32-i386")

base_address = 0x100000;
user_address = 0x400000;

SECTIONS
{
	. = user_address;

	.user : AT(user_address)
	{
		_user_start = .;
		build/user.o
		. = ALIGN(4096);
		_user_end = .;
	}

	. = base_address;
	_ro_start = .;

	.text : AT(base_address)
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
	_ro_end = .;

	_rw_start = .;
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
	_rw_end = .;

	_stack_start = .;
	.stack :
	{
		_stack_bot = .; 
		. += 8192;
		_stack_top = .;
		. = ALIGN(4096);
	}
	_stack_end = .;
}