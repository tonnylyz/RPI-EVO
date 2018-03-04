#include "lib.h"

extern struct Env *env;

#define PAGE_FAULT_TEMP (U_XSTACK_TOP - 2 * BY2PG)

static void pgfault(unsigned long va)
{
    int r;
    unsigned long pte;
    va = (va >> PGSHIFT) << PGSHIFT;
    pte = vpt[PDX(va) * 512 * 512 + PMX(va) * 512 + PTX(va)];
    if (!(pte & PTE_COW)) {
        user_panic("pgfault : not copy on write");
    }
    r = syscall_mem_alloc(0, PAGE_FAULT_TEMP, PTE_USER | PTE_RW);
    if (r < 0) {
        user_panic("pgfault : syscall_mem_alloc");
    }
    user_bcopy((void *)va, (void *)PAGE_FAULT_TEMP, BY2PG);
    r = syscall_mem_map(0, PAGE_FAULT_TEMP, 0, va, PTE_USER | PTE_RW);
    if (r < 0) {
        user_panic("pgfault : syscall_mem_map");
    }
    r = syscall_mem_unmap(0, PAGE_FAULT_TEMP);
    if (r < 0) {
        user_panic("pgfault : syscall_mem_unmap");
    }
}

extern void __asm_pgfault_handler(void);

static void duppage(unsigned int envid, unsigned long va, unsigned long pte)
{
    int r;
    unsigned long perm;
    perm = pte & 0xFFF;
    if (pte & PTE_LIBRARY) {
        r = syscall_mem_map(0, va, envid, va, perm & PTE_LIBRARY);
        if (r < 0) {
            writef("[ERR] duppage : syscall_mem_map #-1\n");
        }
    } else if ((perm & PTE_RO) == 0 || (pte & PTE_COW) != 0) {
        r = syscall_mem_map(0, va, envid, va, PTE_USER | PTE_RO | PTE_COW);
        if (r < 0) {
            writef("[ERR] duppage : syscall_mem_map #0\n");
        }
        r = syscall_mem_map(0, va, 0,     va, PTE_USER | PTE_RO | PTE_COW);
        if (r < 0) {
            writef("[ERR] duppage : syscall_mem_map #1\n");
        }
    } else {
        r = syscall_mem_map(0, va, envid, va, perm);
        if (r < 0) {
            writef("[ERR] duppage : syscall_mem_map #3\n");
        }
    }
}

int fork() {
    unsigned int envid;
    int ret;
    unsigned long va, i, j, k, pte;

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

    for (i = 0; i < 512; i++) {
        if (vpd[i] == 0) continue;
        for (j = 0; j < 512; j++) {
            if (vpm[i * 512 + j] == 0) continue;
            for (k = 0; k < 512; k++) {
                pte = vpt[i * 512 * 512 + j * 512 + k];
                if (pte == 0) continue;
                va = (i * 512 * 512 + j * 512 + k) << PGSHIFT;
                if (va >= U_STACK_TOP - BY2PG) goto DUPPAGE_OK;
                duppage(envid, va, pte);
            }
        }
    }

    DUPPAGE_OK:
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
