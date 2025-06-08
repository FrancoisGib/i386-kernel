#ifndef __IDT_H__
#define __IDT_H__

#include <stdint.h>

struct regs
{
    unsigned int gs, fs, es, ds;                         /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax; /* pushed by 'pusha' */
    unsigned int int_no, err_code;                       /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags, useresp, ss;           /* pushed by the processor automatically */
};

void init_idt(void);
void set_irq_handler(uint8_t irq_no, void *handler);
void set_int_handler(uint8_t int_no, void *handler, uint8_t dpl);
void set_fault_handler(uint8_t fault_no, void *handler);

#endif // __IDT_H__