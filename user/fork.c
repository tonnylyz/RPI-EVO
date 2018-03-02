#include "lib.h"

extern struct Env *env;

#define PAGE_FAULT_TEMP (U_XSTACK_TOP - 2 * BY2PG)

static void pgfault(unsigned long va)
{
    int r;
    unsigned long pte = syscall_pgtable_entry(va);
    va = (va >> PGSHIFT) << PGSHIFT;
    if (!(pte & PTE_COW)) {
        user_panic("pgfault : not copy on write");
    }
    r = syscall_mem_alloc(0, PAGE_FAULT_TEMP, PTE_USER | PTE_RW);
    if (r < 0) {
        user_panic("pgfault : syscall_mem_alloc");
    }
    user_bcopy(va, PAGE_FAULT_TEMP, BY2PG);
    r = syscall_mem_map(0, PAGE_FAULT_TEMP, 0, va, PTE_USER | PTE_RW);
    if (r < 0) {
        user_panic("pgfault : syscall_mem_map");
    }
    r = syscall_mem_unmap(0, PAGE_FAULT_TEMP);
    if (r < 0) {
        user_panic("pgfault : syscall_mem_unmap");
    }
    syscall_set_trapframe(0, 0); // Resume trap frame
}

extern void __asm_pgfault_handler(void);

extern struct Env *envs;
extern struct Env *env;

int fork() {
    unsigned int envid;
    int ret;

    set_pgfault_handler(pgfault);
    envid = syscall_env_alloc();
    if (envid < 0) {
        user_panic("[ERR] fork %d : syscall_env_alloc failed", ret);
        return -1;
    }
    if (envid == 0) {
        env = &(envs[ENVX(syscall_getenvid())]);
        return 0;
    }

    unsigned long va;
    for (va = 0; va < U_STACK_TOP - BY2PG; va += BY2PG) {
        unsigned long pte = syscall_pgtable_entry(va);
        if (pte == 0) continue;
        syscall_mem_map(0, va, envid, va, PTE_USER | PTE_RO | PTE_COW);
        syscall_mem_map(0, va, 0,     va, PTE_USER | PTE_RO | PTE_COW);
    }

    ret = syscall_mem_alloc(envid, U_XSTACK_TOP - BY2PG, PTE_USER | PTE_RW);
    if (ret < 0) {
        user_panic("[ERR] fork %d : syscall_mem_alloc failed", envid);
        return ret;
    }
    ret = syscall_set_pgfault_handler(envid, __asm_pgfault_handler, U_XSTACK_TOP);
    if (ret < 0) {
        user_panic("[ERR] fork %d : syscall_set_pgfault_handler failed", envid);
        return ret;
    }
    ret = syscall_set_env_status(envid, ENV_RUNNABLE);
    if (ret < 0) {
        user_panic("[ERR] fork %d : syscall_set_env_status failed", envid);
        return ret;
    }
    return envid;
}
