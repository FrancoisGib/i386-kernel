#include "lib.h"
#include "gdt.h"
#include "idt.h"

extern __attribute__((fastcall)) void switch_user(uint32_t stack_top);

extern char *user_stack_top;

void timer_irq(void)
{
    printf("timer\n");
    outb(0x20, 0x20);
    // __asm__ volatile("sti");
    for (;;)
        ;
}

void main(void)
{
    init_screen();
    init_gdt();
    init_idt();
    printf("Hello World !\n");
    // __asm__ volatile("sti");
    switch_user((uint32_t)user_stack_top);
    // __asm__ volatile("hlt");
    for (;;)
        ;
}
