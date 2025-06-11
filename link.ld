ENTRY(_start)
OUTPUT_FORMAT("elf32-i386")

kernel_phys_address = 0x100000;
user_address = 0x40000000;

kernel_virt_address = 0xC0000000;

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

	
	. = kernel_phys_address;
	.boot : AT(kernel_phys_address)
	{
		_boot_start = .;

		*(.multiboot)
		build/crt0.o (.text .rodata)
		build/boot.o (.text .rodata)
		build/screen.o
		build/lib.o
		build/mmu.o

		. = ALIGN(4096);
		_boot_end = .;
	}

	.boot_rw : 
	{
		_boot_rw_start = .;
		
		build/crt0.o
		build/boot.o

		. = ALIGN(4096);
		_boot_stack_bot = .;
		. += 4096;
		_boot_stack_top = .;

		_boot_rw_end = .;
	}


	_kernel_lma_start = kernel_phys_address + SIZEOF(.boot) + SIZEOF(.boot_rw);

	. = kernel_virt_address;
	_ro_start = .;

	.text : AT(_kernel_lma_start)
	{
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

	.stack :
	{
		_kernel_stack_bot = .; 
		. += 8192;
		. = ALIGN(4096);
		_kernel_stack_top = .;
	}

	/DISCARD/ :
	{
    	*(.eh_frame)
    	*(.eh_frame_hdr)
	}
}