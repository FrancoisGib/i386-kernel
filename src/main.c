#include "lib.h"
#include "gdt.h"

extern __attribute__((fastcall)) void switch_user(uint32_t stack_top);

extern char *user_stack_top;

void main(void)
{
    init_screen();
    init_gdt();
    printf("Hello World !");
    switch_user((uint32_t)user_stack_top);
}
