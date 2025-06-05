#include "lib.h"
#include "gdt.h"
#include "idt.h"

extern __attribute__((fastcall)) void switch_user(uint32_t stack_top);

extern char *user_stack_top;

void divide_by_zero_handler(void)
{
    printf("division by zero\n");
    __asm__ volatile("hlt");
}

void main(void)
{
    init_screen();
    init_gdt();
    init_idt();
    printf("Hello World !\n");

    // simuler une division par zero
    __asm__ volatile(
        "mov %0, %%eax\n"
        "mov $0, %%edx\n"
        "div %1\n"
        :
        : "r"(0), "r"(0)
        : "eax", "edx");

    printf("after\n");
    // switch_user((uint32_t)user_stack_top);
}
