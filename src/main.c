#include "lib.h"
// #include "gdt.h"
// #include "idt.h"
#include "mmu.h"
#include "keyboard.h"

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
    // init_screen();
    // init_key_map();
    // // init_gdt();
    // // init_idt();
    // init_mmu();

    // set_irq_handler(0x20, timer_irq);
    // set_irq_handler(0x21, keyboard_handler);
    // set_int_handler(0x80, syscall_handler, 3);
    // set_fault_handler(0xE, page_fault_handler);

    // enable_mmu();

    // printf("Hello World !\n");
    // __asm__ volatile("sti");
    // switch_user((uint32_t)user_stack_top);
    for (;;)
        ;
}
