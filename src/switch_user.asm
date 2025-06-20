extern user_main

global switch_user
switch_user:
	mov ax, (4 * 8) | 3 ; ring 3 data with bottom 2 bits set for ring 3
	mov ds, ax
	mov es, ax 
	mov fs, ax 
	mov gs, ax ; SS is handled by iret

	; set up the stack frame iret expects
	mov eax, esp
	push (4 * 8) | 3 ; data selector
	push ecx ; user stack (first param)
	pushf ; eflags
	push (3 * 8) | 3 ; code selector (ring 3 code with bottom 2 bits set for ring 3)
	push user_main
	iret
