#include "idt.h"
#include "gdt.h"
#include "lib.h"
#include <stdint.h>

#define IDT_ENTRIES_NUMBER 256

#define OFFSET_LOW(offset) (((uint32_t)offset) & 0xFFFF)
#define P(p) ((p) & 0b1)
#define DPL(dpl) ((dpl) & 0b11)
#define GATE_TYPE(gate_type) ((gate_type) & 0xF)
#define OFFSET_HIGH(offset) ((((uint32_t)offset) >> 16) & 0xFFFF)
#define SEGMENT_SELECTOR(segment_selector) (segment_selector)
#define TYPE_ATTRIBUTES(gate_type, dpl, p) \
    ((((P(p) << 7) | (DPL(dpl) << 5) | GATE_TYPE(gate_type)) & ~0x10))

#define IDT_ENTRY(offset, selector, gate_type, dpl)                     \
    (idt_entry_t)                                                       \
    {                                                                   \
        .offset_low = OFFSET_LOW(offset),                               \
        .segment_selector = selector,                                   \
        .type_attributes = (uint8_t)TYPE_ATTRIBUTES(gate_type, dpl, 1), \
        .offset_high = OFFSET_HIGH(offset),                             \
        .reserved = 0,                                                  \
    }

typedef struct
{
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t reserved;
    uint8_t type_attributes;
    uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;

idt_entry_t idt[IDT_ENTRIES_NUMBER];

struct
{
    uint16_t size;
    uint32_t offset;
} __attribute__((packed)) idtr;

#define FAULT_HANDLER(i, message) \
    void fault_handler_##i(void)  \
    {                             \
        printf(message);          \
        __asm__ volatile("hlt");  \
    }

#define FAULT_HANDLER_STATUS(i, message)                \
    void fault_handler_##i(void)                        \
    {                                                   \
        uint32_t cr2 UNUSED;                            \
        __asm__ volatile("movl %0, %%cr2" : "=r"(cr2)); \
        printf("%s | CR2 : %x", message, cr2);          \
        __asm__ volatile("hlt");                        \
    }

FAULT_HANDLER(0x0, "Divide Error")
FAULT_HANDLER(0x5, "BOUND Range Exceeded")
FAULT_HANDLER(0x6, "Invalid Opcode (Undefined Opcode)")
FAULT_HANDLER(0x7, "Device Not Available (No Math Coprocessor)")
FAULT_HANDLER_STATUS(0x8, "Double Fault")
FAULT_HANDLER(0x9, "Coprocessor Segment Overrun (reserved)")
FAULT_HANDLER_STATUS(0xA, "Invalid TSS")
FAULT_HANDLER_STATUS(0xB, "Segment Not Present")
FAULT_HANDLER_STATUS(0xC, "Stack-Segment Fault")
FAULT_HANDLER_STATUS(0xD, "General Protection")
FAULT_HANDLER_STATUS(0xE, "Page Fault")
FAULT_HANDLER(0x10, "x87 FPU Floating-Point Error (Math Fault)")
FAULT_HANDLER_STATUS(0x11, "Alignment Check")
FAULT_HANDLER(0x12, "Machine Check")
FAULT_HANDLER(0x13, "SIMD Floating-Point Exception")
FAULT_HANDLER(0x14, "Virtualization Exception")
FAULT_HANDLER_STATUS(0x15, "Control Protection Exception")

#define FAULT_ENTRY(i) idt[i] = IDT_ENTRY(fault_handler_##i, KERNEL_CODE_SELECTOR, 0xE, 0)

extern void timer_irq(void);

void remap_irq(void)
{
    outb(0x20, 0x11); /* write ICW1 to PICM, we are gonna write commands to PICM */
    outb(0xA0, 0x11); /* write ICW1 to PICS, we are gonna write commands to PICS */
    outb(0x21, 0x20); /* remap PICM to 0x20 (32 decimal) */
    outb(0xA1, 0x28); /* remap PICS to 0x28 (40 decimal) */
    outb(0x21, 0x04); /* IRQ2 -> connection to slave */
    outb(0xA1, 0x02);
    outb(0x21, 0x01); /* write ICW4 to PICM, we are gonna write commands to PICM */
    outb(0xA1, 0x01); /* write ICW4 to PICS, we are gonna write commands to PICS */
    outb(0x21, 0x0);  /* enable all IRQs on PICM */
    outb(0xA1, 0x0);  /* enable all IRQs on PICS */
}

void syscall_handler(void)
{
    printf("ok\n");
    for (;;)
        ;
}

struct regs
{
    unsigned int gs, fs, es, ds;                         /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax; /* pushed by 'pusha' */
    unsigned int int_no, err_code;                       /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags, useresp, ss;           /* pushed by the processor automatically */
};

extern void _isr0(void);

void init_idt(void)
{
    remap_irq();
    memset(&idt, 0, sizeof(idt));
    FAULT_ENTRY(0x0);
    FAULT_ENTRY(0x5);
    FAULT_ENTRY(0x6);
    FAULT_ENTRY(0x7);
    FAULT_ENTRY(0x8);
    FAULT_ENTRY(0x9);
    FAULT_ENTRY(0xA);
    FAULT_ENTRY(0xB);
    FAULT_ENTRY(0xC);
    FAULT_ENTRY(0xD);
    FAULT_ENTRY(0xE);
    FAULT_ENTRY(0x10);
    FAULT_ENTRY(0x11);
    FAULT_ENTRY(0x12);
    FAULT_ENTRY(0x13);
    FAULT_ENTRY(0x14);
    FAULT_ENTRY(0x15);

    idt[0x80] = IDT_ENTRY(_isr0, KERNEL_CODE_SELECTOR, 0xE, 3);
    idt[0x20] = IDT_ENTRY(timer_irq, KERNEL_CODE_SELECTOR, 0xE, 0);

    idtr.size = sizeof(idt) - 1;
    idtr.offset = (uint32_t)&idt;

    __asm__ volatile("lidt %0" ::"m"(idtr));
}

void _fault_handler(struct regs *r)
{
    /* Is this a fault whose number is from 0 to 31? */
    if (r->int_no < 32)
    {
        /* Display the description for the Exception that occurred.
         *  In this tutorial, we will simply halt the system using an
         *  infinite loop */
        // puts(exception_messages[r->int_no]);
        puts(" Exception. System Halted!\n");
        for (;;)
            ;
    }
    printf("int\n");
}