extern isr_default_handler
extern global_fault_handler
extern global_irq_handler
extern global_int_handler

.L_GDT_KERNEL_CODE_SEGMENT equ 0x10

global _isr_default_handler
_isr_default_handler:
    cli
    push 0
    push 255
    mov ecx, isr_default_handler
    jmp isr_handler

%macro FAULT_HANDLER_ERR_CODE 1
global fault_handler_%1
fault_handler_%1:
    cli
    push %1
    mov ecx, global_fault_handler
    jmp isr_handler
%endmacro

%macro HANDLER 2
global %1_handler_%2
%1_handler_%2:
    cli
    push 0
    push %2
    mov ecx, global_%1_handler
    jmp isr_handler
%endmacro

%macro FAULT_HANDLER_NO_ERR_CODE 1
HANDLER fault, %1
%endmacro

%macro IRQ_HANDLER 1
HANDLER irq, %1
%endmacro

%macro INT_HANDLER 1
HANDLER int, %1
%endmacro

FAULT_HANDLER_NO_ERR_CODE 0x0
FAULT_HANDLER_NO_ERR_CODE 0x1
FAULT_HANDLER_NO_ERR_CODE 0x2
FAULT_HANDLER_NO_ERR_CODE 0x3
FAULT_HANDLER_NO_ERR_CODE 0x4
FAULT_HANDLER_NO_ERR_CODE 0x5
FAULT_HANDLER_NO_ERR_CODE 0x6
FAULT_HANDLER_NO_ERR_CODE 0x7
FAULT_HANDLER_NO_ERR_CODE 0x8 ; err code already pushed
FAULT_HANDLER_NO_ERR_CODE 0x9
FAULT_HANDLER_ERR_CODE 0xA
FAULT_HANDLER_ERR_CODE 0xB
FAULT_HANDLER_ERR_CODE 0xC
FAULT_HANDLER_ERR_CODE 0xD
FAULT_HANDLER_ERR_CODE 0xE
FAULT_HANDLER_NO_ERR_CODE 0xF
FAULT_HANDLER_NO_ERR_CODE 0x10
FAULT_HANDLER_ERR_CODE 0x11
FAULT_HANDLER_NO_ERR_CODE 0x12
FAULT_HANDLER_NO_ERR_CODE 0x13
FAULT_HANDLER_NO_ERR_CODE 0x14
FAULT_HANDLER_NO_ERR_CODE 0x15
FAULT_HANDLER_NO_ERR_CODE 0x1C
FAULT_HANDLER_ERR_CODE 0x1D
FAULT_HANDLER_ERR_CODE 0x1E

IRQ_HANDLER 0x0
IRQ_HANDLER 0x1

INT_HANDLER 0x80

isr_handler:
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
    push esp       ; Push the stack pointer
    call ecx       ; A special call, preserves the 'eip' register
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8     ; Cleans up the pushed error code and pushed ISR number
    iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!