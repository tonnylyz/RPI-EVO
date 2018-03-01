#ifndef OSLABPI_MMU_H
#define OSLABPI_MMU_H

#define MAXPA 0x3F000000

#define BY2PG        4096
#define PDMAP        (4 * 1024 * 1024)
#define PDSHIFT        30
#define PMSHIFT        21
#define PGSHIFT        12

#define PDX(va) ((((unsigned long)(va)) >> 30) & 0x1FF)
#define PMX(va) ((((unsigned long)(va)) >> 21) & 0x1FF)
#define PTX(va) ((((unsigned long)(va)) >> 12) & 0x1FF)

#define PTE_ADDR(pte) ((unsigned long)(pte)& 0x7FFFFFF000UL)

#define PPN(va) (((unsigned long)(va)) >> 12)
#define VPN(va) (((unsigned long)(va) & 0x7FFFFFFFFF) >> 12)

#define KADDR(pa) ((unsigned long)(pa) | 0xFFFFFF8000000000UL)

#define KERNEL_LIMIT_PA 0x02000000
#define KERNEL_TSTOP_PA 0x01800000
#define KERNEL_ENVS_PA  0x01400000
#define KERNEL_PAGES_PA 0x01000000
#define KERNEL_PGDIR_PA 0x00200000
#define KERNEL_BASE_PA  0x00080000
#define KERNEL_STACK_PA 0x00080000

#define KTOP         KADDR(KERNEL_LIMIT_PA)
#define TIMESTACKTOP KADDR(KERNEL_TSTOP_PA)
#define KERNEL_ENVS  KADDR(KERNEL_ENVS_PA)
#define KERNEL_PAGES KADDR(KERNEL_PAGES_PA)
#define KERNEL_PGDIR KADDR(KERNEL_PGDIR_PA)
#define KERNBASE     KADDR(KERNEL_BASE_PA)
#define KSTACKTOP    KADDR(KERNEL_STACK_PA)

#define UTOP         (0xffffffff)
#define USTACKTOP    (0x01000000)
#define UENVS        (0x90000000)
#define UPAGES       (0xa0000000)

#define PTE_NORMAL      (0 << 2)
#define PTE_DEVICE      (1 << 2)
#define PTE_NON_CACHE   (2 << 2)

#define PTE_4KB         (0b11)
#define PTE_KERN        (0 << 6)
#define PTE_USER        (1 << 6)
#define PTE_RW          (0 << 7)
#define PTE_RO          (1 << 7)
#define PTE_AF          (1 << 10)
#define PTE_PXN         (1UL << 53)
#define PTE_UXN         (1UL << 54)
#define PTE_OUTER_SHARE (2 << 8)
#define PTE_INNER_SHARE (3 << 8)


#define assert(x) do { if (!(x)) panic("assertion failed: %s", #x); } while (0)

#endif //OSLABPI_MMU_H
