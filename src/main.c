#include "lib.h"
#include "gdt.h"

void user_main(void)
{
    puts("User mode !");
    for (;;)
        ;
}

extern void switch_user(void);

void main(void)
{
    init_gdt();
    init_screen();
    puts("Hello World !");
    switch_user();
}