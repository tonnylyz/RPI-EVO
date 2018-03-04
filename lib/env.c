#include <mmu.h>
#include <error.h>
#include <env.h>
#include <kerelf.h>
#include <sched.h>
#include <pmap.h>
#include <printf.h>

struct Env *envs = (struct Env *)K_ENVS_BASE;
struct Env *curenv;

static struct Env_list env_free_list;

u_int mkenvid(struct Env *e) {
    static u_long next_env_id = 0;
    u_int idx = e - envs;
    return (++next_env_id << (1 + LOG2NENV)) | idx;
}

int envid2env(u_int envid, struct Env **penv, int checkperm) {
    struct Env *e;
    if (envid == 0) {
        *penv = curenv;
        return 0;
    }
    e = &envs[ENVX(envid)];
    if (e->env_status == ENV_FREE || e->env_id != envid) {
        *penv = 0;
        return -E_BAD_ENV;
    }
    if (checkperm && e != curenv && e->env_parent_id != curenv->env_id) {
        *penv = 0;
        return -E_BAD_ENV;
    }
    *penv = e;
    return 0;
}

void env_init(void) {
    LIST_INIT(&env_free_list);
    int i;
    for (i = NENV - 1; i >= 0; i--) {
        envs[i].env_id = i;
        envs[i].env_status = ENV_FREE;
        LIST_INSERT_HEAD(&env_free_list, &envs[i], env_link);
    }
    curenv = NULL;
}

static int env_setup_vm(struct Env *e) {
    int r;
    struct Page *p = NULL;
    if ((r = page_alloc(&p)) < 0) {
        panic("env_setup_vm - page_alloc error\n");
    }
    p->pp_ref++;
    e->env_pgdir = (u_long *) page2kva(p);

    map_segment(e->env_pgdir, U_ENVS_BASE, ROUND(sizeof(struct Env) * NENV, BY2PG), P_ENVS_BASE, PTE_USER | PTE_RO);
    map_segment(e->env_pgdir, U_PAGES_BASE, ROUND(sizeof(struct Page) * (P_LIMIT / BY2PG), BY2PG), P_PAGES_BASE, PTE_USER | PTE_RO);

    page_insert(e->env_pgdir, pa2page(PADDR(e->env_pgdir)), UVPD, PTE_USER | PTE_RO);
    return 0;
}

int env_alloc(struct Env **newenv, u_int parent_id) {
    int r;
    struct Env *e;
    e = LIST_FIRST(&env_free_list);
    if (e == NULL) {
        panic("env_alloc : E_NO_FREE_ENV err\n");
    }
    r = env_setup_vm(e);
    if (r < 0) {
        panic("env_alloc : env_setup_vm err %d\n", r);
    }
    e->env_id = mkenvid(e);
    e->env_parent_id = parent_id;
    e->env_status = ENV_RUNNABLE;

    e->env_tf.spsr = 0;
    e->env_tf.sp = U_STACK_TOP;

    LIST_REMOVE(e, env_link);
    *newenv = e;
    return 0;
}

static int load_icode_mapper(u_long va, u_int sgsize, u_char *bin, u_int bin_size, void *user_data) {
    struct Env *env = (struct Env *) user_data;
    struct Page *p = NULL;
    u_long i;
    int r;
    u_long offset = va - ROUNDDOWN(va, BY2PG);
    for (i = 0; i < bin_size; i += BY2PG) {
        r = page_alloc(&p);
        if (r < 0) {
            panic("load_icode_mapper : Allocate page failed.");
        }
        p->pp_ref++;
        r = page_insert(env->env_pgdir, p, va - offset + i, PTE_USER | PTE_RW);
        if (r < 0) {
            panic("load_icode_mapper : Insert page failed.");
        }
        bcopy(bin + i, (void *) page2kva(p) + offset, MIN(BY2PG, bin_size - i));
    }
    while (i < sgsize) {
        r = page_alloc(&p);
        if (r < 0) {
            panic("load_icode_mapper : Allocate page failed.");
        }
        p->pp_ref++;
        r = page_insert(env->env_pgdir, p, va - offset + i, PTE_USER | PTE_RW);
        if (r < 0) {
            panic("load_icode_mapper : Insert page failed.");
        }
        i += BY2PG;
    }
    return 0;
}

static void load_icode(struct Env *e, u_char *binary, u_int size) {
    struct Page *p = NULL;
    u_long entry_point;
    u_long r;
    r = page_alloc(&p);
    if (r < 0) {
        panic("load_icode : Allocate page failed.");
    }
    r = page_insert(e->env_pgdir, p, U_STACK_TOP - BY2PG, PTE_USER | PTE_RW);
    if (r < 0) {
        panic("load_icode : Insert page failed.");
    }
    r = load_elf(binary, size, &entry_point, e, load_icode_mapper);
    if (r < 0) {
        printf("load_icode : Load elf failed.");
    }
    e->env_tf.elr = entry_point;
}

void env_create(u_char *binary, unsigned int size) {
    struct Env *e;
    env_alloc(&e, 0);
    load_icode(e, binary, size);
}

void env_free(struct Env *e) {
    Pte *pme;
    Pte *pte;
    u_long pdeno, pmeno, pteno;
    printf("[%08x] free env %08x\n", curenv ? curenv->env_id : 0, e->env_id);
    for (pdeno = 0; pdeno < PDX(U_LIMIT); pdeno++) {
        if (!(e->env_pgdir[pdeno] & PTE_4KB)) {
            continue;
        }
        pme = (Pte *)KADDR(PTE_ADDR(e->env_pgdir));
        for (pmeno = 0; pmeno < PMX(U_LIMIT); pmeno++) {
            if (!(pme[pmeno] & PTE_4KB)) {
                continue;
            }
            pte = (Pte *)KADDR(PTE_ADDR(pme[pmeno]));
            for (pteno = 0; pteno < PTX(U_LIMIT); pteno++) {
                if (pte[pteno] & PTE_4KB) {
                    page_remove(e->env_pgdir, (pdeno << PDSHIFT) | (pmeno << PMSHIFT) | (pteno << PGSHIFT));
                }
            }
            page_decref(pa2page(PTE_ADDR(pme[pmeno])));
            pme[pmeno] = 0;
        }
        page_decref(pa2page(PTE_ADDR(e->env_pgdir[pdeno])));
        e->env_pgdir[pdeno] = 0;
    }
    page_decref(pa2page((u_long) e->env_pgdir));
    bzero(e, sizeof(struct Env));
    e->env_status = ENV_FREE;
    LIST_INSERT_HEAD(&env_free_list, e, env_link);
}

void env_destroy(struct Env *e) {
    printf("env_destroy called\n");
    env_free(e);
    if (curenv == e) {
        curenv = NULL;
        sched_yield();
    }
}

void set_ttbr0(u_long pa) {
    asm volatile ("msr ttbr0_el1, %0" :: "r" (pa));
}

void tlb_invalidate() {
    asm volatile ("dsb ishst; tlbi vmalle1is; dsb ish; isb");
}

void env_run(struct Env *e) {
    struct Trapframe *old = (struct Trapframe *) (K_TIMESTACK_TOP - sizeof(struct Trapframe));
    if (curenv) {
        bcopy(old, &(curenv->env_tf), sizeof(struct Trapframe));
    }
    curenv = e;
    bcopy(&curenv->env_tf, old, sizeof(struct Trapframe));
    set_ttbr0(((u_long)curenv->env_pgdir) & 0xFFFFFFFF);
    tlb_invalidate();
}