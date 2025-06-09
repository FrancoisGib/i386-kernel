#include "mmu.h"

#define PAGE_SIZE 4096
#define NUM_ENTRIES 1024
#define NUM_DIRECTORIES 8
#define NB_PAGES (NUM_ENTRIES * NUM_DIRECTORIES)
#define STACK_SIZE (PAGE_SIZE * NUM_ENTRIES / 4) // 1 mo

#define KERNEL_DIR 0
#define USER_DIR 1

#define KERNEL_MODE 0
#define USER_MODE 1

#define RO_MODE 0
#define RW_MODE 1

#define CR0_PG 0x80000000

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

#define SET_CR3(pd) ({                            \
    __asm__ volatile("movl %0, %%cr3" ::"r"(pd)); \
})

#define PAGE_TO_ADDR(page) ((void *)((uintptr_t)page << 12))
#define ADDR_TO_PAGE(addr) ((uint32_t)((uintptr_t)addr >> 12))

typedef struct
{
    uint8_t valid : 1;          // 1 valid, 0 invalid
    uint8_t write_access : 1;   // 1 read/write, 0 read only
    uint8_t access_mode : 1;    // 0 user mode, 1 kernel mode
    uint8_t cache_defer : 1;    // 0 write-through, 1 write-back (defer)
    uint8_t cache_disabled : 1; // 0 cache enabled, 1 cache disabled
    uint8_t used : 1;           // 1 if the page has been read
    uint8_t _pad2 : 1;
    uint8_t size : 1; // 0 => 4Ko, 1 => 4Mo
    uint8_t _pad1 : 4;
    uint32_t page_table : 20; // 20 bits page address
} __attribute__((packed)) directory_entry_t;

typedef struct
{
    uint8_t valid : 1;          // 1 valid, 0 invalid
    uint8_t write_access : 1;   // 1 read/write, 0 read only
    uint8_t access_mode : 1;    // 1 user mode, 0 kernel mode
    uint8_t cache_defer : 1;    // 0 write-through, 1 write-back (defer)
    uint8_t cache_disabled : 1; // 0 cache enabled, 1 cache disabled
    uint8_t read : 1;           // 1 if the page has been read
    uint8_t dirty : 1;          // 1 if the page has been written
    uint8_t _pad2 : 1;
    uint8_t global : 1; // 1 if the page is global
    uint8_t _pad1 : 3;
    uint32_t physical_page : 20; // 20 bits page entry
} __attribute__((packed)) page_entry_t;

directory_entry_t page_directory[NUM_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
page_entry_t page_tables[NB_PAGES] __attribute__((aligned(PAGE_SIZE)));

directory_entry_t *page_directory_ptr = page_directory;
int32_t first_free_page = 0;
int32_t pages[NB_PAGES];

void init_pages(void)
{
    for (int i = 0; i < NB_PAGES - 1; i++)
    {
        pages[i] = i + 1;
    }
    pages[NB_PAGES - 1] = -1;
}

void *alloc_page(void)
{
    uint32_t page = first_free_page;
    if (first_free_page == -1)
    {
        return NULL;
    }
    first_free_page = pages[page];
    pages[page] = -1;
    return PAGE_TO_ADDR(page);
}

void free_page(void *page_address)
{
    uint32_t page = ADDR_TO_PAGE(page_address);
    pages[page] = first_free_page;
    first_free_page = page;
}

void page_copy(char *pg_src, char *pg_dst)
{
    for (int i = 0; i < PAGE_SIZE; i++)
    {
        pg_dst[i] = pg_src[i];
    }
}

static void setup_page_directory(uint16_t directory_index, uint8_t access_mode, uint8_t write_access)
{
    page_entry_t *page_table = &page_tables[directory_index * NUM_ENTRIES];
    page_directory[directory_index].valid = 1;
    page_directory[directory_index].cache_disabled = 1;
    page_directory[directory_index].write_access = write_access;
    page_directory[directory_index].access_mode = access_mode;
    page_directory[directory_index].page_table = ADDR_TO_PAGE(page_table);
}

static void setup_identity_page_range(uint32_t begin, uint32_t end, uint8_t access_mode, uint8_t write_access)
{
    for (uint32_t i = begin; i < end; i++)
    {
        page_tables[i].valid = 1;
        page_tables[i].cache_disabled = 1;
        page_tables[i].access_mode = access_mode;
        page_tables[i].write_access = write_access;
        page_tables[i].physical_page = i;
    }
}

// static void setup_page_range(uint32_t begin, uint32_t end, uint32_t phys_addr, uint8_t access_mode, uint8_t write_access)
// {
//     for (uint32_t i = begin; i < end; i++)
//     {
//         void *page = alloc_page();
//         uint32_t virtual_page = ADDR_TO_PAGE(page);
//         page_tables[virtual_page].valid = 1;
//         page_tables[virtual_page].cache_disabled = 1;
//         page_tables[virtual_page].access_mode = access_mode;
//         page_tables[virtual_page].write_access = write_access;
//         page_tables[virtual_page].physical_page = ADDR_TO_PAGE(phys_addr + (i * PAGE_SIZE));
//     }
// }

void init_mmu(void)
{
    init_pages();
    extern char _ro_start;
    extern char _ro_end;
    uint32_t kernel_ro_start_page = ADDR_TO_PAGE(&_ro_start);
    uint32_t kernel_ro_end_page = ADDR_TO_PAGE(&_ro_end);

    extern char _rw_start;
    extern char _rw_end;
    uint32_t kernel_rw_start_page = ADDR_TO_PAGE(&_rw_start);
    uint32_t kernel_rw_end_page = ADDR_TO_PAGE(&_rw_end);

    extern char _kernel_stack_bot;
    extern char _kernel_stack_top;
    uint32_t kernel_stack_start_page = ADDR_TO_PAGE(&_kernel_stack_bot);
    uint32_t kernel_stack_end_page = ADDR_TO_PAGE(&_kernel_stack_top);

    // Setup kernel page directory
    setup_page_directory(KERNEL_DIR, KERNEL_MODE, RW_MODE);
    // setup_identity_page_range(0x100, 0x120, KERNEL_MODE, RO_MODE);

    setup_identity_page_range(kernel_ro_start_page, kernel_ro_end_page, KERNEL_MODE, RO_MODE);
    setup_identity_page_range(kernel_rw_start_page, kernel_rw_end_page, KERNEL_MODE, RW_MODE);
    setup_identity_page_range(kernel_stack_start_page, kernel_stack_end_page, KERNEL_MODE, RW_MODE);

    setup_identity_page_range(ADDR_TO_PAGE(0xB8000), ADDR_TO_PAGE((0xB8000 + (25 * 80))) + 1, KERNEL_MODE, RW_MODE);
    SET_CR3(page_directory);
}

void enable_mmu(void)
{
    MMU_ENABLE();
}

void disable_mmu(void)
{
    MMU_DISABLE();
}

void *get_cr2(void)
{
    void *cr2;
    __asm__ volatile("movl %%cr2, %0" : "=r"(cr2));
    return cr2;
}

void page_fault_handler(struct regs *r)
{
    void *cr2 = get_cr2();
    printf("Memory fault at address : %x, instruction : %x, err : %x\n", cr2, r->eip, r->err_code);
    for (;;)
        ;
}