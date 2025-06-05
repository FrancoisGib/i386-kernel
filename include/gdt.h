#ifndef __GDT_H__
#define __GDT_H__

#define KERNEL_RPL 0
#define USER_RPL 3
#define TI 0

#define GDT_KERNEL_CODE_INDEX 1
#define GDT_KERNEL_DATA_INDEX 2
#define GDT_USER_CODE_INDEX 3
#define GDT_USER_DATA_INDEX 4
#define GDT_TSS_INDEX 5

#define KERNEL_CODE_SELECTOR ((GDT_KERNEL_CODE_INDEX << 3) | (TI << 2) | KERNEL_RPL)
#define KERNEL_DATA_SELECTOR ((GDT_KERNEL_DATA_INDEX << 3) | (TI << 2) | KERNEL_RPL)

#define USER_CODE_SELECTOR ((GDT_USER_CODE_INDEX << 3) | (TI << 2) | USER_RPL)
#define USER_DATA_SELECTOR ((GDT_USER_DATA_INDEX << 3) | (TI << 2) | USER_RPL)
#define TSS_SELECTOR ((GDT_TSS_INDEX << 3) | (TI << 2) | KERNEL_RPL)

void init_gdt(void);

#endif // __GDT_H__