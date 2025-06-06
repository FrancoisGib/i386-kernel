extern global_fault_handler
extern global_irq_handler
extern global_int_handler

.L_GDT_KERNEL_CODE_SEGMENT equ 0x10

; double fault handler
global fault_handler_0x8
fault_handler_0x8:
    cli
    pop byte edx                    ; first fault
    push byte 0x8
    mov ecx, global_fault_handler
    jmp idt_global_handler

%macro FAULT_HANDLER 1
global fault_handler_%1
fault_handler_%1:
    cli
    push byte %1
    mov ecx, global_fault_handler
    jmp idt_global_handler
%endmacro

%macro IRQ_HANDLER 1
global irq_handler_%1
irq_handler_%1:
    cli
    push byte %1
    mov ecx, global_irq_handler
    jmp idt_global_handler
%endmacro

%macro INT_HANDLER 1
global int_handler_%1
int_handler_%1:
    cli
    push byte %1
    mov ecx, global_int_handler
    jmp idt_global_handler
%endmacro

FAULT_HANDLER 0x0
FAULT_HANDLER 0x5
FAULT_HANDLER 0x6
FAULT_HANDLER 0x7
FAULT_HANDLER 0x9
FAULT_HANDLER 0xA
FAULT_HANDLER 0xB
FAULT_HANDLER 0xC
FAULT_HANDLER 0xD
FAULT_HANDLER 0xE
FAULT_HANDLER 0x10
FAULT_HANDLER 0x11
FAULT_HANDLER 0x12
FAULT_HANDLER 0x13
FAULT_HANDLER 0x14
FAULT_HANDLER 0x15

IRQ_HANDLER 0x20

INT_HANDLER 0x80

idt_global_handler:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, .L_GDT_KERNEL_CODE_SEGMENT   ; Load the Kernel Data Segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp       ; Push the stack
    mov eax, ecx
    call eax       ; A special call, preserves the 'eip' register
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 4     ; Cleans up the pushed error code and pushed ISR number
    iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!