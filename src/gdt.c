#include "gdt.h"

#define GDT_SEGMENTS_NUMBER 6

#define GDT_TSS_INDEX 5
#define RPL 0
#define TI 0
#define TSS_SELECTOR ((GDT_TSS_INDEX << 3) | (TI << 2) | RPL)

#define GDT_KERNEL_DATA_INDEX 2
#define KERNEL_DATA_SELECTOR ((GDT_KERNEL_DATA_INDEX << 3) | (TI << 2) | RPL)

typedef struct
{
    uint16_t limit_low; // limit 0 - 15
    uint16_t base_low;  // base 0 - 15
    uint8_t base_mid;   // base 16 - 23
    uint8_t access;
    uint8_t limit_flags; // limit 16 - 19 and flags
    uint8_t base_high;   // base 24 - 31
} __attribute__((packed, aligned(4))) gdt_entry_t;

gdt_entry_t gdt[GDT_SEGMENTS_NUMBER];

typedef struct
{
    uint16_t prev_task_link;
    uint16_t reserved_0;
    uint32_t esp0;
    uint16_t ss0;
    uint16_t reserved_1;
    uint32_t esp1;
    uint16_t ss1;
    uint16_t reserved_2;
    uint32_t esp2;
    uint16_t ss2;
    uint16_t reserved_3;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint16_t es;
    uint16_t reserved_4;
    uint16_t cs;
    uint16_t reserved_5;
    uint16_t ss;
    uint16_t reserved_6;
    uint16_t ds;
    uint16_t reserved_7;
    uint16_t fs;
    uint16_t reserved_8;
    uint16_t gs;
    uint16_t reserved_9;
    uint16_t ldt_segment_selector;
    uint16_t reserved_10;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed, aligned(4))) tss_t;

tss_t tss;

struct
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdtr;

void init_tss(uint32_t stack_ptr, uint16_t data_selector)
{
    memset(&tss, 0, sizeof(tss));

    tss.ss0 = data_selector;
    tss.esp0 = stack_ptr;

    tss.cs = 0x08;
    tss.ss = 0x10;
    tss.ds = 0x10;
    tss.es = 0x10;
    tss.fs = 0x10;
    tss.gs = 0x10;

    tss.iomap_base = sizeof(tss);

    gdtr.limit = sizeof(gdt_entry_t) * GDT_SEGMENTS_NUMBER - 1;
    gdtr.base = (uint32_t)&gdt;

    __asm__ volatile("lgdt %0" : : "m"(gdtr));
    // __asm__ volatile("ltr %w0" : : "r"(TSS_SELECTOR));
}

/**
 * @brief Constructs the access byte in a GDT entry with the different parameters.
 *
 * @param present Specifies if the descriptor is valid or not.
 * @param ring Sets the descriptor ring level. (0 to 3 with 0 being the highest privilege level)
 * @param descriptor_type Indicates whether the descriptor is a segment (code, data) or system descriptor (TSS, LDT...).
 * @param segment_type Sets the permissions on a segment or the type on a system descriptor.
 *
 * @return The constructed access byte to be used in a GDT entry
 */
uint8_t get_access(descriptor_presence_t present, uint8_t ring, descriptor_type_t descriptor_type, uint8_t segment_type)
{
    return ((present & 0b1) << 7) | ((ring & 0b11) << 5) | ((descriptor_type & 0b1) << 4) | (segment_type & 0xF);
}

/**
 * @brief Constructs the flags byte for the GDT limit field and returns combined limit flags.
 *
 * @param limit The segment limit (20 bits, max = 0xFFFFF).
 * @param available Indicates if the segment is available or not.
 * @param default_operation_size Sets default operation size (16 or 32 bit).
 * @param long_mode Sets if descriptor is for long mode.
 * @param granularity Sets the granularity (byte or 4 kilobytes pages).
 *
 * @return The combined 4 flags' bits and 4 limit's high weight bits.
 */
uint8_t get_limit_flags(uint32_t limit, available_t available, default_op_size_t default_operation_size, long_mode_t long_mode, granularity_t granularity)
{
    uint8_t flags = ((granularity & 0b1) << 3) | ((default_operation_size & 0b1) << 2) | ((long_mode & 0b1) << 1) | (available & 1);
    return ((limit >> 16) & 0x0F) | (flags << 4);
}

gdt_entry_t get_entry(uint32_t base_address, uint32_t limit, uint8_t ring, descriptor_type_t descriptor_type, uint8_t segment_type, granularity_t granularity)
{
    gdt_entry_t entry;
    entry.limit_low = limit & 0xFFFF;
    entry.base_low = base_address & 0xFFFF;
    entry.base_mid = (base_address >> 0x10) & 0xFF;
    entry.access = get_access(DESCRIPTOR_PRESENT, ring, descriptor_type, segment_type);
    entry.limit_flags = get_limit_flags(limit, AVAILABLE_FALSE, DEFAULT_OPERATION_SIZE_32, LONG_MODE_I386, granularity);
    entry.base_high = (base_address >> 0x18) & 0xFF;
    return entry;
}

extern char _stack_top;

void init_gdt(void)
{
    gdt[0] = (gdt_entry_t){0, 0, 0, 0, 0, 0};
    gdt[1] = get_entry(0x0, 0xFFFFF, 0, DESCRIPTOR_TYPE_SEGMENT, SEG_TYPE_CODE_EXECUTE_READ, GRANULARITY_PAGE);
    gdt[2] = get_entry(0x0, 0xFFFFF, 0, DESCRIPTOR_TYPE_SEGMENT, SEG_TYPE_DATA_READ_WRITE, GRANULARITY_PAGE);
    gdt[3] = get_entry(0x0, 0xFFFFF, 3, DESCRIPTOR_TYPE_SEGMENT, SEG_TYPE_CODE_EXECUTE_READ, GRANULARITY_PAGE);
    gdt[4] = get_entry(0x0, 0xFFFFF, 3, DESCRIPTOR_TYPE_SEGMENT, SEG_TYPE_DATA_READ_WRITE, GRANULARITY_PAGE);
    gdt[5] = get_entry((uint32_t)&tss, sizeof(tss) - 1, 0, DESCRIPTOR_TYPE_SYSTEM, SYS_SEG_TYPE_TSS_AVAILABLE, GRANULARITY_BYTE);
    init_tss((uint32_t)&_stack_top - 4, KERNEL_DATA_SELECTOR);
}
