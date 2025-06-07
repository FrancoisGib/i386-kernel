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

void main(void)
{
    init_screen();
    init_gdt();
    init_idt();
    printf("Hello World !\n");
    __asm__ volatile("sti");
    switch_user((uint32_t)user_stack_top);
}
