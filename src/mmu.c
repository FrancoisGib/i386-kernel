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
#define ADDR_TO_PAGE(addr) ((uint16_t)((uintptr_t)addr >> 12))

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
int16_t first_free_page = 0;
int16_t pages[NB_PAGES];

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

static void setup_page_range(uint16_t begin, uint16_t end, uint8_t access_mode, uint8_t write_access)
{
    for (uint16_t i = begin; i < end; i++)
    {
        void *page = alloc_page();
        page_tables[i].valid = 1;
        page_tables[i].cache_disabled = 1;
        page_tables[i].access_mode = access_mode;
        page_tables[i].write_access = write_access;
        page_tables[i].physical_page = ADDR_TO_PAGE(page);
    }
}

void init_mmu(void)
{
    init_pages();

    // Setup kernel page directory
    setup_page_directory(KERNEL_DIR, KERNEL_MODE, RW_MODE);
    setup_page_range(0, NUM_ENTRIES, KERNEL_MODE, RW_MODE);

    SET_CR3(page_directory);
    MMU_ENABLE();
}

void enable_mmu(void)
{
    MMU_ENABLE();
}

void disable_mmu(void)
{
    MMU_DISABLE();
}