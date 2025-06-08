#include "lib.h"
#include "gdt.h"
#include "idt.h"

extern __attribute__((fastcall)) void switch_user(uint32_t stack_top);

void syscall_handler(struct regs *r)
{
    printf("syscall\n");
    r->eax = 1;
}

extern char *user_stack_top;

void timer_irq(void)
{
    printf("timer\n");
}

void keyboard_irq(struct regs *r)
{
    (void)r;
    unsigned char scancode;

    scancode = inb(0x60);
    (void)scancode;
    printf("keyboard\n");
}

void main(void)
{
    init_screen();
    init_gdt();
    init_idt();

    set_irq_handler(0x20, timer_irq);
    set_irq_handler(0x21, keyboard_irq);
    set_int_handler(0x80, syscall_handler, 3);

    printf("Hello World !\n");
    // __asm__ volatile("sti");
    switch_user((uint32_t)user_stack_top);
}
