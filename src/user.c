char user_stack[8192] __attribute__((aligned(4096)));
char *user_stack_top = &user_stack[8192];

void user_main(void)
{
    unsigned int res = 0;
    __asm__ volatile("int $0x80; mov %%eax, %0" : "=r"(res));
    for (;;)
        ;
}