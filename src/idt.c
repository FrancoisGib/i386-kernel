#include "idt.h"
#include "gdt.h"
#include "lib.h"

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

#define IRQ_HANDLER(i)                 \
    extern void irq_handler_##i(void); \
    idt[i] = IDT_ENTRY(irq_handler_##i, KERNEL_CODE_SELECTOR, 0xE, 0);

#define INT_HANDLER(i)                 \
    extern void int_handler_##i(void); \
    idt[i] = IDT_ENTRY(int_handler_##i, KERNEL_CODE_SELECTOR, 0xE, 0);

void *fault_handlers[25];
void *irq_handlers[16];
void *int_handlers[208]; // 256 - 32 - 16

void init_idt(void)
{
    remap_irq();
    memset(&idt, 0, sizeof(idt));
    memset(fault_handlers, 0, sizeof(fault_handlers));
    memset(irq_handlers, 0, sizeof(irq_handlers));
    memset(int_handlers, 0, sizeof(int_handlers));

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

    IRQ_HANDLER(0x20);
    IRQ_HANDLER(0x21);
    IRQ_HANDLER(0x22);
    IRQ_HANDLER(0x23);
    IRQ_HANDLER(0x24);
    IRQ_HANDLER(0x25);
    IRQ_HANDLER(0x26);
    IRQ_HANDLER(0x27);
    IRQ_HANDLER(0x70);
    IRQ_HANDLER(0x71);
    IRQ_HANDLER(0x72);
    IRQ_HANDLER(0x73);
    IRQ_HANDLER(0x74);
    IRQ_HANDLER(0x75);
    IRQ_HANDLER(0x76);
    IRQ_HANDLER(0x77);

    INT_HANDLER(0x28);
    INT_HANDLER(0x29);
    INT_HANDLER(0x2A);
    INT_HANDLER(0x2B);
    INT_HANDLER(0x2C);
    INT_HANDLER(0x2D);
    INT_HANDLER(0x2E);
    INT_HANDLER(0x2F);
    INT_HANDLER(0x30);
    INT_HANDLER(0x31);
    INT_HANDLER(0x32);
    INT_HANDLER(0x33);
    INT_HANDLER(0x34);
    INT_HANDLER(0x35);
    INT_HANDLER(0x36);
    INT_HANDLER(0x37);
    INT_HANDLER(0x38);
    INT_HANDLER(0x39);
    INT_HANDLER(0x3A);
    INT_HANDLER(0x3B);
    INT_HANDLER(0x3C);
    INT_HANDLER(0x3D);
    INT_HANDLER(0x3E);
    INT_HANDLER(0x3F);
    INT_HANDLER(0x40);
    INT_HANDLER(0x41);
    INT_HANDLER(0x42);
    INT_HANDLER(0x43);
    INT_HANDLER(0x44);
    INT_HANDLER(0x45);
    INT_HANDLER(0x46);
    INT_HANDLER(0x47);
    INT_HANDLER(0x48);
    INT_HANDLER(0x49);
    INT_HANDLER(0x4A);
    INT_HANDLER(0x4B);
    INT_HANDLER(0x4C);
    INT_HANDLER(0x4D);
    INT_HANDLER(0x4E);
    INT_HANDLER(0x4F);
    INT_HANDLER(0x50);
    INT_HANDLER(0x51);
    INT_HANDLER(0x52);
    INT_HANDLER(0x53);
    INT_HANDLER(0x54);
    INT_HANDLER(0x55);
    INT_HANDLER(0x56);
    INT_HANDLER(0x57);
    INT_HANDLER(0x58);
    INT_HANDLER(0x59);
    INT_HANDLER(0x5A);
    INT_HANDLER(0x5B);
    INT_HANDLER(0x5C);
    INT_HANDLER(0x5D);
    INT_HANDLER(0x5E);
    INT_HANDLER(0x5F);
    INT_HANDLER(0x60);
    INT_HANDLER(0x61);
    INT_HANDLER(0x62);
    INT_HANDLER(0x63);
    INT_HANDLER(0x64);
    INT_HANDLER(0x65);
    INT_HANDLER(0x66);
    INT_HANDLER(0x67);
    INT_HANDLER(0x68);
    INT_HANDLER(0x69);
    INT_HANDLER(0x6A);
    INT_HANDLER(0x6B);
    INT_HANDLER(0x6C);
    INT_HANDLER(0x6D);
    INT_HANDLER(0x6E);
    INT_HANDLER(0x6F);
    INT_HANDLER(0x78);
    INT_HANDLER(0x79);
    INT_HANDLER(0x7A);
    INT_HANDLER(0x7B);
    INT_HANDLER(0x7C);
    INT_HANDLER(0x7D);
    INT_HANDLER(0x7E);
    INT_HANDLER(0x7F);
    INT_HANDLER(0x80);
    INT_HANDLER(0x81);
    INT_HANDLER(0x82);
    INT_HANDLER(0x83);
    INT_HANDLER(0x84);
    INT_HANDLER(0x85);
    INT_HANDLER(0x86);
    INT_HANDLER(0x87);
    INT_HANDLER(0x88);
    INT_HANDLER(0x89);
    INT_HANDLER(0x8A);
    INT_HANDLER(0x8B);
    INT_HANDLER(0x8C);
    INT_HANDLER(0x8D);
    INT_HANDLER(0x8E);
    INT_HANDLER(0x8F);
    INT_HANDLER(0x90);
    INT_HANDLER(0x91);
    INT_HANDLER(0x92);
    INT_HANDLER(0x93);
    INT_HANDLER(0x94);
    INT_HANDLER(0x95);
    INT_HANDLER(0x96);
    INT_HANDLER(0x97);
    INT_HANDLER(0x98);
    INT_HANDLER(0x99);
    INT_HANDLER(0x9A);
    INT_HANDLER(0x9B);
    INT_HANDLER(0x9C);
    INT_HANDLER(0x9D);
    INT_HANDLER(0x9E);
    INT_HANDLER(0x9F);
    INT_HANDLER(0xA0);
    INT_HANDLER(0xA1);
    INT_HANDLER(0xA2);
    INT_HANDLER(0xA3);
    INT_HANDLER(0xA4);
    INT_HANDLER(0xA5);
    INT_HANDLER(0xA6);
    INT_HANDLER(0xA7);
    INT_HANDLER(0xA8);
    INT_HANDLER(0xA9);
    INT_HANDLER(0xAA);
    INT_HANDLER(0xAB);
    INT_HANDLER(0xAC);
    INT_HANDLER(0xAD);
    INT_HANDLER(0xAE);
    INT_HANDLER(0xAF);
    INT_HANDLER(0xB0);
    INT_HANDLER(0xB1);
    INT_HANDLER(0xB2);
    INT_HANDLER(0xB3);
    INT_HANDLER(0xB4);
    INT_HANDLER(0xB5);
    INT_HANDLER(0xB6);
    INT_HANDLER(0xB7);
    INT_HANDLER(0xB8);
    INT_HANDLER(0xB9);
    INT_HANDLER(0xBA);
    INT_HANDLER(0xBB);
    INT_HANDLER(0xBC);
    INT_HANDLER(0xBD);
    INT_HANDLER(0xBE);
    INT_HANDLER(0xBF);
    INT_HANDLER(0xC0);
    INT_HANDLER(0xC1);
    INT_HANDLER(0xC2);
    INT_HANDLER(0xC3);
    INT_HANDLER(0xC4);
    INT_HANDLER(0xC5);
    INT_HANDLER(0xC6);
    INT_HANDLER(0xC7);
    INT_HANDLER(0xC8);
    INT_HANDLER(0xC9);
    INT_HANDLER(0xCA);
    INT_HANDLER(0xCB);
    INT_HANDLER(0xCC);
    INT_HANDLER(0xCD);
    INT_HANDLER(0xCE);
    INT_HANDLER(0xCF);
    INT_HANDLER(0xD0);
    INT_HANDLER(0xD1);
    INT_HANDLER(0xD2);
    INT_HANDLER(0xD3);
    INT_HANDLER(0xD4);
    INT_HANDLER(0xD5);
    INT_HANDLER(0xD6);
    INT_HANDLER(0xD7);
    INT_HANDLER(0xD8);
    INT_HANDLER(0xD9);
    INT_HANDLER(0xDA);
    INT_HANDLER(0xDB);
    INT_HANDLER(0xDC);
    INT_HANDLER(0xDD);
    INT_HANDLER(0xDE);
    INT_HANDLER(0xDF);
    INT_HANDLER(0xE0);
    INT_HANDLER(0xE1);
    INT_HANDLER(0xE2);
    INT_HANDLER(0xE3);
    INT_HANDLER(0xE4);
    INT_HANDLER(0xE5);
    INT_HANDLER(0xE6);
    INT_HANDLER(0xE7);
    INT_HANDLER(0xE8);
    INT_HANDLER(0xE9);
    INT_HANDLER(0xEA);
    INT_HANDLER(0xEB);
    INT_HANDLER(0xEC);
    INT_HANDLER(0xED);
    INT_HANDLER(0xEE);
    INT_HANDLER(0xEF);
    INT_HANDLER(0xF0);
    INT_HANDLER(0xF1);
    INT_HANDLER(0xF2);
    INT_HANDLER(0xF3);
    INT_HANDLER(0xF4);
    INT_HANDLER(0xF5);
    INT_HANDLER(0xF6);
    INT_HANDLER(0xF7);
    INT_HANDLER(0xF8);
    INT_HANDLER(0xF9);
    INT_HANDLER(0xFA);
    INT_HANDLER(0xFB);
    INT_HANDLER(0xFC);
    INT_HANDLER(0xFD);
    INT_HANDLER(0xFE);
    INT_HANDLER(0xFF);

    idtr.size = sizeof(idt) - 1;
    idtr.offset = (uint32_t)&idt;

    __asm__ volatile("lidt %0" ::"m"(idtr));
}

void set_dpl(idt_entry_t *entry, uint8_t dpl)
{
    entry->type_attributes = (entry->type_attributes & ~(0b11 << 5)) | (DPL(dpl) << 5);
}

#define FAULT_HANDLER_INDEX(i) ((i > 27 && i < 31) ? (i - 6) : i)
#define IRQ_HANDLER_INDEX(i) ((i < 0x70) ? (i - 0x20) : (i - 0x68))
#define INT_HANDLER_INDEX(i) ((i > 0x27 && i < 0x70) ? (i - 0x28) : (i - 0x30))

void set_irq_handler(uint8_t irq_no, void *handler)
{
    if ((irq_no >= 0x20 && irq_no <= 0x27) || (irq_no >= 0x70 && irq_no <= 0x77))
    {
        irq_handlers[IRQ_HANDLER_INDEX(irq_no)] = handler;
    }
}

void set_int_handler(uint8_t int_no, void *handler, uint8_t dpl)
{
    if (int_no >= 0x28 && (int_no < 0x70 || int_no >= 0x78))
    {
        set_dpl(&idt[int_no], dpl);
        int_handlers[INT_HANDLER_INDEX(int_no)] = handler;
    }
}

void set_fault_handler(uint8_t fault_no, void *handler)
{
    if (fault_no < 31 && !(fault_no >= 21 && fault_no <= 26))
    {
        fault_handlers[FAULT_HANDLER_INDEX(fault_no)] = handler;
    }
}

typedef void (*idt_handler_t)(struct regs *r);

void global_fault_handler(struct regs *r)
{
    uint8_t handler_index = FAULT_HANDLER_INDEX((uint8_t)r->int_no);
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
    uint8_t handler_index = IRQ_HANDLER_INDEX((uint8_t)r->int_no);
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
    else
    {
        printf("Unhandled interrupt : 0x%x\n", r->int_no);
    }
}
