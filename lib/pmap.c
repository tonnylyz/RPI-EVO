#include <env.h>
#include "pmap.h"
struct Page *pages = (struct Page *)K_PAGES_BASE;
static struct Page_list page_free_list;

void page_init(void) {
    LIST_INIT(&page_free_list);
    u_int used_index = P_RESERVED_TOP / BY2PG;
    u_int i;
    for (i = 0; i < used_index; i++) {
        pages[i].pp_ref = 1;
    }
    for (i = used_index; i < P_LIMIT / BY2PG; i++) {
        pages[i].pp_ref = 0;
        LIST_INSERT_HEAD(&page_free_list, &pages[i], pp_link);
    }
}

void bzero(void* b, u_long len) {
    u_long max = (u_long)b + len;
    while ((u_long)b + 7 < max) {
        *(u_long *) b = 0;
        b += 8;
    }
    while ((u_long) b < max) {
        *(u_char *) b++ = 0;
    }
}

void bcopy(const void *src, void *dst, u_long len) {
    u_long max = (u_long)dst + len;
    while ((u_long) dst + 7 < max) {
        *(u_long *) dst = *(u_long *) src;
        dst += 8;
        src += 8;
    }
    while ((u_long) dst < max) {
        *(u_char *) dst = *(u_char *) src;
        dst += 1;
        src += 1;
    }
}

int page_alloc(struct Page **pp) {
    struct Page *page;
    page = LIST_FIRST(&page_free_list);
    if (page == NULL) {
        panic("page_alloc E_NO_MEM");
        return -E_NO_MEM;
    }
    LIST_REMOVE(page, pp_link);
    bzero((void *)page2kva(page), BY2PG);
    page->pp_ref = 0;
    *pp = page;
    return 0;
}

void page_free(struct Page *pp) {
    if (pp->pp_ref > 0)
        return;
    if (pp->pp_ref == 0) {
        LIST_INSERT_HEAD(&page_free_list, pp, pp_link);
        return;
    }
}

int pgdir_walk(Pde *pgdir, u_long va, int create, Pte **ppte) {
    struct Page *ppage;
    u_long *pde;
    u_long *pme;
    u_long *pte;
    pde = (u_long *)KADDR(&pgdir[PDX(va)]);
    if (!(*pde & PTE_4KB)) {
        if (!create) {
            *ppte = NULL;
            return 0;
        }
        page_alloc(&ppage);
        *pde = page2pa(ppage) | PTE_KERN | PTE_RW | PTE_AF | PTE_4KB;

        page_insert(pgdir, ppage, UVPM + (PDX(va) << PGSHIFT), PTE_USER | PTE_RO);

        ppage->pp_ref = 1;
    }
    pme = (u_long *)KADDR(PTE_ADDR(*pde)) + PMX(va);
    if (!(*pme & PTE_4KB)) {
        if (!create) {
            *ppte = NULL;
            return 0;
        }
        page_alloc(&ppage);
        *pme = page2pa(ppage) | PTE_KERN | PTE_RW | PTE_AF | PTE_4KB;

        page_insert(pgdir, ppage, UVPT + (PDX(va) << PMSHIFT) + (PMX(va) << PGSHIFT), PTE_USER | PTE_RO);

        ppage->pp_ref = 1;
    }
    pte = (u_long *)KADDR(PTE_ADDR(*pme)) + PTX(va);
    *ppte = pte;
    return 0;
}

int page_insert(Pde *pgdir, struct Page *pp, u_long va, u_long perm) {
    u_long PERM;
    Pte *pgtable_entry;
    PERM = perm | PTE_NORMAL | PTE_INNER_SHARE | PTE_AF | PTE_4KB;
    pgdir_walk(pgdir, va, 1, &pgtable_entry);
    if ((*pgtable_entry & PTE_4KB) != 0) {
        if (pa2page(*pgtable_entry) != pp) {
            page_remove(pgdir, va);
        } else {
            tlb_invalidate();
            *pgtable_entry = page2pa(pp) | PERM;
            return 0;
        }
    }
    tlb_invalidate();
    if (pgdir_walk(pgdir, va, 1, &pgtable_entry) != 0) {
        panic ("page insert check failed .\n");
    }
    *pgtable_entry = page2pa(pp) | PERM;
    pp->pp_ref++;
    return 0;
}

struct Page *page_lookup(Pde *pgdir, u_long va, Pte **ppte) {
    struct Page *ppage;
    Pte *pte;
    pgdir_walk(pgdir, va, 0, &pte);
    if (pte == 0) {
        return 0;
    }
    if ((*pte & PTE_4KB) == 0) {
        return 0;
    }
    ppage = pa2page(PTE_ADDR(*pte));
    if (ppte) {
        *ppte = pte;
    }
    return ppage;
}

void page_decref(struct Page *pp) {
    if (--pp->pp_ref == 0) {
        page_free(pp);
    }
}

void page_remove(Pde *pgdir, u_long va) {
    Pte *pagetable_entry;
    struct Page *ppage;
    ppage = page_lookup(pgdir, va, &pagetable_entry);
    if (ppage == 0) {
        return;
    }
    ppage->pp_ref--;
    if (ppage->pp_ref == 0) {
        page_free(ppage);
    }
    *pagetable_entry = 0;
    tlb_invalidate();
}

void map_segment(Pde *pgdir, u_long va, u_long size, u_long pa, u_long perm) {
    Pte *pte;
    int i;
    for (i = 0; i < size; i += BY2PG) {
        pgdir_walk(pgdir, va + i, 1, &pte);
        *pte = PTE_ADDR(pa + i) | perm | PTE_NORMAL | PTE_INNER_SHARE | PTE_AF | PTE_4KB;
    }
}