#include "lib.h"

typedef LIST_ENTRY(Page) Page_LIST_entry_t;
struct Page {
    Page_LIST_entry_t pp_link;
    unsigned short pp_ref;
};

int pageref(void *va) {
    u_long pte;
    if (vpd[PDX(va)] == 0 || vpm[PDX(va) * 512 + PMX(va)] == 0) {
        return 0;
    }
    pte = vpt[PDX(va) * 512 * 512 + PMX(va) * 512 + PTX(va)];
    if (pte == 0) {
        return 0;
    }
    return pages[PTE_ADDR(pte) >> PGSHIFT].pp_ref;
}
