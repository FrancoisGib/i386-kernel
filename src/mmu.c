#include "mmu.h"

#define PAGE_SIZE 4096
#define NUM_ENTRIES 1024
#define NUM_DIRECTORIES 1024
#define NB_PAGES NUM_DIRECTORIES

#define KERNEL_DIR 0

#define KERNEL_MODE 0
#define USER_MODE 1

#define RO_MODE 0
#define RW_MODE 1

#define CR0_PG 0x80000000

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

int32_t first_free_page = 0;
int32_t pages[NUM_DIRECTORIES];

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

// void free_page(void *page_address)
// {
//     uint32_t page = ADDR_TO_PAGE(page_address);
//     pages[page] = first_free_page;
//     first_free_page = page;
// }

// void page_copy(char *pg_src, char *pg_dst)
// {
//     for (int i = 0; i < PAGE_SIZE; i++)
//     {
//         pg_dst[i] = pg_src[i];
//     }
// }

static page_entry_t *allocate_page_table(void)
{
    void *page = alloc_page();
    memset(page, 0, sizeof(page_entry_t) * NUM_ENTRIES);
    return (page_entry_t *)page;
}

static void setup_page_directory(uint16_t directory_index, uint8_t access_mode, uint8_t write_access)
{
    page_entry_t *page_table = allocate_page_table();
    page_directory[directory_index].valid = 1;
    page_directory[directory_index].cache_disabled = 1;
    page_directory[directory_index].write_access = write_access;
    page_directory[directory_index].access_mode = access_mode;
    page_directory[directory_index].page_table = ADDR_TO_PAGE(page_table);
}

static void setup_identity_page_range(uint32_t begin, uint32_t end, uint8_t access_mode, uint8_t write_access)
{
    page_entry_t *page_table = (page_entry_t *)PAGE_TO_ADDR((page_directory[begin / NUM_ENTRIES].page_table));
    for (uint32_t i = begin; i < end; i++)
    {
        page_table[i % NUM_ENTRIES].valid = 1;
        page_table[i % NUM_ENTRIES].cache_disabled = 1;
        page_table[i % NUM_ENTRIES].access_mode = access_mode;
        page_table[i % NUM_ENTRIES].write_access = write_access;
        page_table[i % NUM_ENTRIES].physical_page = i;
    }
}

void setup_page_range(uint32_t begin, uint32_t end, uint32_t phys_addr, uint8_t access_mode, uint8_t write_access)
{
    page_entry_t *page_table = (page_entry_t *)PAGE_TO_ADDR((page_directory[begin / NUM_ENTRIES].page_table));
    for (uint32_t i = begin; i < end; i++)
    {
        page_table[i % NUM_ENTRIES].valid = 1;
        page_table[i % NUM_ENTRIES].cache_disabled = 1;
        page_table[i % NUM_ENTRIES].access_mode = access_mode;
        page_table[i % NUM_ENTRIES].write_access = write_access;
        page_table[i % NUM_ENTRIES].physical_page = ADDR_TO_PAGE((phys_addr + i * PAGE_SIZE));
    }
}

void init_mmu(void)
{
    init_pages();
    setup_page_directory(KERNEL_DIR, KERNEL_MODE, RW_MODE);

    extern char _boot_start;
    extern char _boot_end;
    uint32_t kernel_boot_start_page = ADDR_TO_PAGE(&_boot_start);
    uint32_t kernel_boot_end_page = ADDR_TO_PAGE(&_boot_end);
    setup_identity_page_range(kernel_boot_start_page, kernel_boot_end_page, KERNEL_MODE, RO_MODE);

    extern char _boot_rw_start;
    extern char _boot_rw_end;
    uint32_t kernel_boot_rw_start_page = ADDR_TO_PAGE(&_boot_rw_start);
    uint32_t kernel_boot_rw_end_page = ADDR_TO_PAGE(&_boot_rw_end);
    setup_identity_page_range(kernel_boot_rw_start_page, kernel_boot_rw_end_page, KERNEL_MODE, RW_MODE);

    extern char kernel_virt_address;
    uint32_t kernel_addr = (uint32_t)&kernel_virt_address;
    uint16_t kernel_virt_page = kernel_addr / PAGE_SIZE / NUM_ENTRIES;
    printf("kerne %d\n", kernel_virt_page);
    setup_page_directory(kernel_virt_page, KERNEL_MODE, RW_MODE);

    uint32_t phys_kernel_addr = (uint32_t)&_boot_rw_end;

    extern char _ro_start;
    extern char _ro_end;
    uint32_t ro_start_page = ADDR_TO_PAGE(&_ro_start);
    uint32_t ro_end_page = ADDR_TO_PAGE(&_ro_end);
    uint32_t ro_size = &_ro_end - &_ro_start;
    setup_page_range(ro_start_page, ro_end_page, phys_kernel_addr, KERNEL_MODE, RO_MODE);
    printf("ro %x - %x : %x\n", ro_start_page, ro_end_page, phys_kernel_addr);

    extern char _rw_start;
    extern char _rw_end;
    uint32_t rw_start_page = ADDR_TO_PAGE(&_rw_start);
    uint32_t rw_end_page = ADDR_TO_PAGE(&_rw_end);
    uint32_t rw_size = &_rw_end - &_rw_start;
    setup_page_range(rw_start_page, rw_end_page, phys_kernel_addr + ro_size, KERNEL_MODE, RW_MODE);
    printf("rw %x - %x : %x\n", rw_start_page, rw_end_page, phys_kernel_addr + ro_size);

    extern char _kernel_stack_bot;
    extern char _kernel_stack_top;
    uint32_t kernel_stack_start_page = ADDR_TO_PAGE(&_kernel_stack_bot);
    uint32_t kernel_stack_end_page = ADDR_TO_PAGE(&_kernel_stack_top);
    setup_page_range(kernel_stack_start_page, kernel_stack_end_page, phys_kernel_addr + ro_size + rw_size, KERNEL_MODE, RW_MODE);
    printf("rw %x - %x : %x\n", kernel_stack_start_page, kernel_stack_end_page, phys_kernel_addr + ro_size + rw_size);

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