#ifndef __MMU_H__
#define __MMU_H__

#include <stdint.h>
#include "idt.h"
#include "lib.h"

#define MMU_ENABLE() ({                             \
    uint32_t cr0;                                   \
    __asm__ volatile("movl %%cr0, %0" : "=r"(cr0)); \
    cr0 |= CR0_PG;                                  \
    __asm__ volatile("movl %0, %%cr0" ::"r"(cr0));  \
})

#define MMU_DISABLE() ({                            \
    uint32_t cr0;                                   \
    __asm__ volatile("movl %%cr0, %0" : "=r"(cr0)); \
    cr0 &= ~CR0_PG;                                 \
    __asm__ volatile("movl %0, %%cr0" ::"r"(cr0));  \
})

void init_mmu(void);
void init_mmu(void);
void enable_mmu(void);
void disable_mmu(void);
void page_fault_handler(struct regs *r);

#endif // __MMU_H__