#include "mmu.h"
#include "screen.h"

#define CR0_PG 0x80000000

extern void main(void);

void kernel_main(void)
{
    // init_screen();
    init_mmu();
    MMU_ENABLE();
    printf("ici\n");
    __asm__ volatile("movl $_kernel_stack_top, %esp\nmovl $_kernel_stack_top, %ebp");
    main();
    for (;;)
        ;
}
