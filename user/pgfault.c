#include "lib.h"
#include <mmu.h>

extern void (*__pgfault_handler)(unsigned long);

extern void __asm_pgfault_handler(void);

// Kernel return to __asm_pgfault_handler then invoke __pgfault_handler to mount cow page
void set_pgfault_handler(void (*fn)(unsigned long va)) {
    if (__pgfault_handler == 0) {
        if (syscall_mem_alloc(0, U_XSTACK_TOP - BY2PG, PTE_USER | PTE_RW) < 0 ||
            syscall_set_pgfault_handler(0, __asm_pgfault_handler, U_XSTACK_TOP) < 0) {
            writef("cannot set pgfault handler\n");
            return;
        }
    }
    __pgfault_handler = fn;
}