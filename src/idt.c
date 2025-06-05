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

extern void divide_by_zero_handler(void);

void init_idt(void)
{
    memset(&idt, 0, sizeof(idt));
    idt[0] = IDT_ENTRY(divide_by_zero_handler, KERNEL_CODE_SELECTOR, 0xE, 0);
    // idt[0x80] = IDT_ENTRY(isr_handler, KERNEL_CODE_SELECTOR, 0xE, 3);

    idtr.size = sizeof(idt) - 1;
    idtr.offset = (uint32_t)&idt;

    __asm__ volatile("lidt %0" ::"m"(idtr));
}