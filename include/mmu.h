#ifndef __MMU_H__
#define __MMU_H__

#include <stdint.h>
#include "idt.h"
#include "lib.h"

void init_mmu(void);
void enable_mmu(void);
void disable_mmu(void);

#endif // __MMU_H__