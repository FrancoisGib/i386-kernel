char user_stack[8192] __attribute__((aligned(4096)));
char *user_stack_top = &user_stack[8192];

#include "lib.h"

void user_main(void)
{
    __asm__ volatile("int $0x80");
    // printf("sqalut\n");
    for (;;)
        ;
}