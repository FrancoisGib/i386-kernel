#include "gdt.h"
#include <stdint.h>
#include "lib.h"

#define GDT_SEGMENTS_NUMBER 6

#define LIMIT_LOW(limit) ((limit) & 0xFFFF)
#define BASE_LOW(base) ((base) & 0xFFFF)
#define BASE_MID(base) (((base) >> 0x10) & 0xFF)
#define BASE_HIGH(base) (((base) >> 0x18) & 0xFF)

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
#define ACCESS(present, ring, descriptor_type, segment_type) \
    (((present & 0b1) << 7) | ((ring & 0b11) << 5) | ((descriptor_type & 0b1) << 4) | (segment_type & 0xF))

/**
 * @brief Constructs the flags 4 bits of a gdt entry.
 *
 * @param available Available for system use.
 * @param default_operation_size Sets default operation size (16 or 32 bit).
 * @param long_mode Indicates if the descriptor is in long mode (64 bit) or not (32 bit).
 * @param granularity Sets the granularity of the segment, in bytes or pages.
 *
 * @return The constructed flags to be used in a GDT entry
 */
#define FLAGS(available, default_operation_size, long_mode, granularity) \
    (((granularity & 0b1) << 3) | ((default_operation_size & 0b1) << 2) | ((long_mode & 0b1) << 1) | (available & 1))

/**
 * @brief Constructs the flags byte and limit field for a GDT entry.
 *
 * @param limit The segment limit (20 bits, max = 0xFFFFF).
 * @param available Available for system use.
 * @param default_operation_size Sets default operation size (16 or 32 bit).
 * @param long_mode Sets if descriptor is for long mode.
 * @param granularity Sets the granularity (byte or 4 kilobytes pages).
 *
 * @return The combined 4 flags' bits and 4 limit's high weight bits.
 */
#define LIMIT_FLAGS(limit, available, default_operation_size, long_mode, granularity) \
    ((((limit) >> 16) & 0xF) | (FLAGS(available, default_operation_size, long_mode, granularity) << 4))

#define GDT_ENTRY(base_address, limit, ring, descriptor_type, segment_type, granularity)                            \
    (gdt_entry_t)                                                                                                   \
    {                                                                                                               \
        .limit_low = LIMIT_LOW(limit),                                                                              \
        .base_low = BASE_LOW(base_address),                                                                         \
        .base_mid = BASE_MID(base_address),                                                                         \
        .access = ACCESS(DESCRIPTOR_PRESENT, ring, descriptor_type, segment_type),                                  \
        .limit_flags = LIMIT_FLAGS(limit, AVAILABLE_FALSE, DEFAULT_OPERATION_SIZE_32, LONG_MODE_I386, granularity), \
        .base_high = BASE_HIGH(base_address)                                                                        \
    }

typedef enum
{
    DESCRIPTOR_INVALID = 0,
    DESCRIPTOR_PRESENT = 1
} descriptor_presence_t;

typedef enum
{
    DESCRIPTOR_TYPE_SYSTEM = 0,
    DESCRIPTOR_TYPE_SEGMENT = 1
} descriptor_type_t;

typedef enum
{
    SEG_TYPE_DATA_READ_ONLY = 0x0,
    SEG_TYPE_DATA_READ_ONLY_ACCESSED = 0x1,
    SEG_TYPE_DATA_READ_WRITE = 0x2,
    SEG_TYPE_DATA_READ_WRITE_ACCESSED = 0x3,
    SEG_TYPE_DATA_READ_ONLY_EXPAND_DOWN = 0x4,
    SEG_TYPE_DATA_READ_ONLY_EXPAND_DOWN_ACCESSED = 0x5,
    SEG_TYPE_DATA_READ_WRITE_EXPAND_DOWN = 0x6,
    SEG_TYPE_DATA_READ_WRITE_EXPAND_DOWN_ACCESSED = 0x7
} segment_type_data_t;

typedef enum
{
    SEG_TYPE_CODE_EXECUTE_ONLY = 0x8,
    SEG_TYPE_CODE_EXECUTE_ONLY_ACCESSED = 0x9,
    SEG_TYPE_CODE_EXECUTE_READ = 0xA,
    SEG_TYPE_CODE_EXECUTE_READ_ACCESSED = 0xB,
    SEG_TYPE_CODE_EXECUTE_ONLY_CONFORMING = 0xC,
    SEG_TYPE_CODE_EXECUTE_ONLY_CONFORMING_ACCESSED = 0xD,
    SEG_TYPE_CODE_EXECUTE_READ_CONFORMING = 0xE,
    SEG_TYPE_CODE_EXECUTE_READ_CONFORMING_ACCESSED = 0xF
} segment_type_code_t;

typedef enum
{
    SYS_SEG_TYPE_LDT = 0x2,
    SYS_SEG_TYPE_TSS_AVAILABLE = 0x9,
    SYS_SEG_TYPE_TSS_BUSY = 0xB,
    SYS_SEG_TYPE_CALL_GATE_16 = 0x4,
    SYS_SEG_TYPE_CALL_GATE_32 = 0xC,
    SYS_SEG_TYPE_INTERRUPT_GATE_16 = 0x6,
    SYS_SEG_TYPE_INTERRUPT_GATE_32 = 0xE,
    SYS_SEG_TYPE_TRAP_GATE_16 = 0x7,
    SYS_SEG_TYPE_TRAP_GATE_32 = 0xF
} segment_type_system_t;

typedef enum
{
    AVAILABLE_FALSE = 0,
    AVAILABLE_TRUE = 1
} available_t;

typedef enum
{
    DEFAULT_OPERATION_SIZE_16 = 0,
    DEFAULT_OPERATION_SIZE_32 = 1
} default_op_size_t;

typedef enum
{
    LONG_MODE_I386 = 0,
    LONG_MODE_X64 = 1
} long_mode_t;

typedef enum
{
    GRANULARITY_BYTE = 0,
    GRANULARITY_PAGE = 1
} granularity_t;

typedef struct
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t limit_flags;
    uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

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
} __attribute__((packed)) tss_t;

struct
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdtr;

gdt_entry_t gdt[GDT_SEGMENTS_NUMBER] = {
    (gdt_entry_t){0, 0, 0, 0, 0, 0},
    GDT_ENTRY(0, 0x1000, 0, DESCRIPTOR_TYPE_SEGMENT, SEG_TYPE_CODE_EXECUTE_READ, GRANULARITY_PAGE),
    GDT_ENTRY(0, 0x1000, 0, DESCRIPTOR_TYPE_SEGMENT, SEG_TYPE_DATA_READ_WRITE, GRANULARITY_PAGE),
    GDT_ENTRY(0x400000, 0x1000, 3, DESCRIPTOR_TYPE_SEGMENT, SEG_TYPE_CODE_EXECUTE_READ, GRANULARITY_PAGE),
    GDT_ENTRY(0x400000, 0x1000, 3, DESCRIPTOR_TYPE_SEGMENT, SEG_TYPE_DATA_READ_WRITE, GRANULARITY_PAGE),
};

tss_t tss;

void init_tss(uint32_t stack_ptr)
{
    memset(&tss, 0, sizeof(tss));

    tss.esp0 = stack_ptr;
    tss.ss0 = KERNEL_DATA_SELECTOR;

    tss.es = tss.ds = tss.fs = tss.gs = tss.ss = KERNEL_DATA_SELECTOR;
    tss.cs = KERNEL_CODE_SELECTOR;
    tss.iomap_base = sizeof(tss);
}

void init_gdt(void)
{
    gdt_entry_t *tss_gdt_entry = &gdt[GDT_TSS_INDEX];
    uint32_t tss_address = (uint32_t)&tss;
    uint32_t limit = sizeof(tss) - 1;

    tss_gdt_entry->access = ACCESS(DESCRIPTOR_PRESENT, 0, DESCRIPTOR_TYPE_SYSTEM, SYS_SEG_TYPE_TSS_AVAILABLE);
    tss_gdt_entry->base_high = BASE_HIGH(tss_address);
    tss_gdt_entry->base_mid = BASE_MID(tss_address);
    tss_gdt_entry->base_low = BASE_LOW(tss_address);
    tss_gdt_entry->limit_low = LIMIT_LOW(limit);
    tss_gdt_entry->limit_flags = LIMIT_FLAGS(limit, AVAILABLE_FALSE, DEFAULT_OPERATION_SIZE_32, LONG_MODE_I386, GRANULARITY_BYTE);

    extern char _stack_top;
    init_tss((uint32_t)&_stack_top);

    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (uint32_t)&gdt;

    __asm__ volatile(
        " lgdt %0         \n"
        " movw %2, %%ax   \n"
        " movw %%ax, %%ds \n"
        " movw %%ax, %%es \n"
        " movw %%ax, %%fs \n"
        " movw %%ax, %%gs \n"
        " movw %%ax, %%ss \n"
        " ljmp %3, $1f    \n" // Far jump to reload CS
        " 1:              \n"
        " ltr %w1         \n"
        :
        : "m"(gdtr), "r"(TSS_SELECTOR), "i"(KERNEL_DATA_SELECTOR), "i"(KERNEL_CODE_SELECTOR)
        : "ax");
}