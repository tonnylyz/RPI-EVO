#include <syscall_all.h>
#include <uart.h>
#include <mmu.h>
#include <env.h>
#include <printf.h>
#include <pmap.h>
#include <sched.h>

extern struct Env *curenv;

void sys_set_return(unsigned long r) {
    struct Trapframe *tf = (struct Trapframe *)(K_TIMESTACK_TOP - sizeof(struct Trapframe));
    tf->regs[0] = (u_long) r;
}

void sys_putchar(int sysno, char c) {
    uart_send_kern((unsigned int) c);
}

unsigned int sys_getenvid() {
    return curenv->env_id;
}

void sys_yield() {
    sched_yield();
}

int sys_env_destroy(int sysno, unsigned int envid) {
    int r;
    struct Env *e;
    if ((r = envid2env(envid, &e, 1)) < 0) {
        return r;
    }
    env_destroy(e);
    return 0;
}

int sys_set_pgfault_handler(int sysno, unsigned int envid, unsigned long func, unsigned long xstacktop) {
    struct Env *env;
    int ret;
    ret = envid2env(envid, &env, 0);
    if (ret < 0)
        return ret;
    env->env_pgfault_handler = func;
    env->env_xstacktop = xstacktop;
    return 0;
}

int sys_mem_alloc(int sysno, unsigned int envid, unsigned long va, unsigned long perm) {
    struct Env *env;
    struct Page *ppage;
    int ret;
    if (va >= U_LIMIT) {
        printf("[ERR] sys_mem_alloc va\n");
        return -1;
    }
    ret = envid2env(envid, &env, 0);
    if (ret < 0) {
        printf("[ERR] sys_mem_alloc envid2env\n");
        return ret;
    }
    ret = page_alloc(&ppage);
    if (ret < 0) {
        printf("[ERR] sys_mem_alloc page_alloc\n");
        return ret;
    }
    bzero((void *)page2kva(ppage), BY2PG);
    ret = page_insert(env->env_pgdir, ppage, va, perm | PTE_USER | PTE_RW);
    if (ret < 0) {
        printf("[ERR] sys_mem_alloc page_insert\n");
        return ret;
    }
    return 0;
}

int sys_mem_map(int sysno, unsigned int srcid, unsigned long srcva, unsigned int dstid, unsigned long dstva, unsigned long perm) {
    int ret;
    struct Env *srcenv;
    struct Env *dstenv;
    struct Page *ppage;
    Pte *ppte;
    srcva = ROUNDDOWN(srcva, BY2PG);
    dstva = ROUNDDOWN(dstva, BY2PG);
    ret = envid2env(srcid, &srcenv, 1);
    if (ret < 0) {
        printf("[ERR] sys_mem_map envid2env\n");
        return ret;
    }
    ret = envid2env(dstid, &dstenv, 1);
    if (ret < 0) {
        printf("[ERR] sys_mem_map envid2env\n");
        return ret;
    }
    ppage = page_lookup(srcenv->env_pgdir, srcva, &ppte);
    if (ppage == NULL) {
        printf("[ERR] sys_mem_map : page_lookup : [%l016x] : [%l016x] : [%l016x]\n", srcenv->env_pgdir, srcva, &ppte);
        return -1;
    }
    ret = page_insert(dstenv->env_pgdir, ppage, dstva, perm);
    if (ret < 0) {
        printf("[ERR] sys_mem_map page_insert\n");
        return ret;
    }
    return 0;
}

int sys_mem_unmap(int sysno, unsigned int envid, unsigned long va) {
    int ret;
    struct Env *env;
    ret = envid2env(envid, &env, 1);
    if (ret < 0) {
        printf("[ERR] sys_mem_unmap : envid2env\n");
        return ret;
    }
    page_remove(env->env_pgdir, va);
    return ret;
}

unsigned int sys_env_alloc() {
    int r;
    struct Env *e;
    r = env_alloc(&e, curenv->env_id);
    if (r < 0) {
        printf("[ERR] sys_env_alloc : env_alloc\n");
        return r;
    }
    bcopy((void *)(K_TIMESTACK_TOP - sizeof(struct Trapframe)), &e->env_tf, sizeof(struct Trapframe));

    Pte *ppte = NULL;
    pgdir_walk(curenv->env_pgdir, U_STACK_TOP - BY2PG, 0, &ppte);
    if (ppte != NULL) {
        struct Page *ppc, *ppp;
        ppp = pa2page(PTE_ADDR(*ppte));
        page_alloc(&ppc);
        bcopy((const void *) page2kva(ppp), (void *) page2kva(ppc), BY2PG);
        page_insert(e->env_pgdir, ppc, U_STACK_TOP - BY2PG, PTE_USER | PTE_RW);
    }

    e->env_status = ENV_NOT_RUNNABLE;
    e->env_tf.regs[0] = 0;
    e->env_ipc_recving = 0;
    return e->env_id;
}

int sys_set_env_status(int sysno, unsigned int envid, unsigned int status) {
    struct Env *env;
    int ret;
    if (status != ENV_RUNNABLE && status != ENV_NOT_RUNNABLE && status != ENV_FREE) {
        return -E_INVAL;
    }
    ret = envid2env(envid, &env, 0);
    if (ret < 0)
        return ret;
    env->env_status = status;
    return 0;
}

void sys_set_trapframe(int sysno, unsigned int envid, struct Trapframe *tf) {
    assert("sys_set_trapframe not implemented.");
}

void sys_panic(int sysno, char *msg) {
    panic("%s", msg);
}

void sys_ipc_recv(int sysno, unsigned long dstva) {
    curenv->env_ipc_dstva = dstva;
    curenv->env_ipc_recving = 1;
    curenv->env_status = ENV_NOT_RUNNABLE;
    sys_yield();
}

int sys_ipc_can_send(int sysno, unsigned int envid, unsigned long value, unsigned long srcva, unsigned long perm) {
    int r;
    struct Env *e;
    r = envid2env(envid, &e, 0);
    if (r < 0) {
        printf("[ERR] sys_ipc_can_send E_BAD_ENV\n");
        return -E_BAD_ENV;
    }
    if (!e->env_ipc_recving) {
        return -E_IPC_NOT_RECV;
    }
    e->env_ipc_recving = 0;
    e->env_ipc_from = curenv->env_id;
    e->env_ipc_value = value;
    e->env_status = ENV_RUNNABLE;
    return 0;
}

char sys_cgetc() {
    return uart_getc_kern();
}
