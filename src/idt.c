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

char *error_messages[] = {
    "Division By Zero",
    "Debug Exception",
    "Non Maskable Interrupt Exception",
    "Breakpoint",
    "Into Detected Overflow Exception",
    "BOUND Range Exceeded",
    "Invalid Opcode (Undefined Opcode)",
    "Device Not Available (No Math Coprocessor)",
    "Double Fault",
    "Coprocessor Segment Overrun (reserved)",
    "Invalid TSS Exception",
    "Segment Not Present Exception",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault Exception",
    "Alignment Check Exception",
    "Machine Check Exception",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    NULL,
};

#define FAULT_HANDLER(i)                 \
    extern void fault_handler_##i(void); \
    idt[i] = IDT_ENTRY(fault_handler_##i, KERNEL_CODE_SELECTOR, 0xE, 0);

#define CUSTOM_FAULT_HANDLER(i, handler)                                 \
    extern void fault_handler_##i(void);                                 \
    idt[i] = IDT_ENTRY(fault_handler_##i, KERNEL_CODE_SELECTOR, 0xE, 0); \
    fault_handlers[i] = handler;

#define IRQ_HANDLER_INDEX(i) ((i < 8) ? (i + 0x20) : (i + 0x68))

#define IRQ_HANDLER(i, handler)                                                           \
    extern void irq_handler_##i(void);                                                    \
    idt[IRQ_HANDLER_INDEX(i)] = IDT_ENTRY(irq_handler_##i, KERNEL_CODE_SELECTOR, 0xE, 0); \
    irq_handlers[i] = handler;

#define INT_HANDLER_INDEX(i) ((i > 0x27 && i < 0x70) ? (i - 0x28) : (i - 0x30))

#define INT_HANDLER(i, handler)                                        \
    extern void int_handler_##i(void);                                 \
    idt[i] = IDT_ENTRY(int_handler_##i, KERNEL_CODE_SELECTOR, 0xE, 3); \
    int_handlers[INT_HANDLER_INDEX(i)] = handler;

void *fault_handlers[25];
void *irq_handlers[16];
void *int_handlers[208]; // 256 - 32 - 16

extern void syscall_handler(struct regs *r);

void keyboard(struct regs *r)
{
    (void)r;
    unsigned char scancode;

    scancode = inb(0x60);
    (void)scancode;
    printf("keyboard\n");
}

void init_idt(void)
{
    remap_irq();
    memset(&idt, 0, sizeof(idt));
    memset(fault_handlers, 0, sizeof(fault_handlers));
    memset(irq_handlers, 0, sizeof(irq_handlers));
    memset(int_handlers, 0, sizeof(int_handlers));
    extern void _isr_default_handler(void);
    for (uint16_t i = 0; i < 256; i++)
    {
        idt[i] = IDT_ENTRY(_isr_default_handler, KERNEL_CODE_SELECTOR, 0xE, 0);
    }

    FAULT_HANDLER(0x0);
    FAULT_HANDLER(0x1);
    FAULT_HANDLER(0x2);
    FAULT_HANDLER(0x3);
    FAULT_HANDLER(0x4);
    FAULT_HANDLER(0x5);
    FAULT_HANDLER(0x6);
    FAULT_HANDLER(0x7);
    FAULT_HANDLER(0x8);
    FAULT_HANDLER(0x9);
    FAULT_HANDLER(0xA);
    FAULT_HANDLER(0xB);
    FAULT_HANDLER(0xC);
    FAULT_HANDLER(0xD);
    FAULT_HANDLER(0xE);
    FAULT_HANDLER(0xF);
    FAULT_HANDLER(0x10);
    FAULT_HANDLER(0x11);
    FAULT_HANDLER(0x12);
    FAULT_HANDLER(0x13);
    FAULT_HANDLER(0x14);
    FAULT_HANDLER(0x15);
    FAULT_HANDLER(0x1C);
    FAULT_HANDLER(0x1D);
    FAULT_HANDLER(0x1E);

    IRQ_HANDLER(0x0, timer_irq);
    IRQ_HANDLER(0x1, keyboard);
    INT_HANDLER(0x80, syscall_handler);

    idtr.size = sizeof(idt) - 1;
    idtr.offset = (uint32_t)&idt;

    __asm__ volatile("lidt %0" ::"m"(idtr));
}

typedef void (*idt_handler_t)(struct regs *r);

void global_fault_handler(struct regs *r)
{
    uint8_t handler_index = (uint8_t)r->int_no;
    if (fault_handlers[handler_index] != NULL)
    {
        idt_handler_t handler = fault_handlers[handler_index];
        handler(r);
        return;
    }

    printf("Error caught: 0x%x", r->err_code);
    if (r->int_no < (sizeof(error_messages) / sizeof(char *)) && error_messages[r->int_no] != NULL)
    {
        printf(", %s\n", error_messages[r->int_no]);
    }
    for (;;)
        ;
}

void global_irq_handler(struct regs *r)
{
    uint8_t handler_index = (uint8_t)r->int_no;
    if (irq_handlers[handler_index] != NULL)
    {
        idt_handler_t handler = irq_handlers[handler_index];
        handler(r);
    }

    if (r->int_no >= 0x70)
    {
        outb(0xA0, 0x20);
    }

    /* In either case, we need to send an EOI to the master
     *  interrupt controller too */
    outb(0x20, 0x20);
}

void global_int_handler(struct regs *r)
{
    uint8_t handler_index = INT_HANDLER_INDEX((uint8_t)r->int_no);
    if (int_handlers[handler_index] != NULL)
    {
        idt_handler_t handler = int_handlers[handler_index];
        handler(r);
    }
}

void isr_default_handler(struct regs *r UNUSED)
{
    printf("Unhandled interrupt\n");
}
