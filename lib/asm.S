
// void set_ttbr0(u64 paddr)
//  set ttbr0 (low address page table base register)
.global set_ttbr0
set_ttbr0:
    msr ttbr0_el1, x0
    ret

// void tlb_invalidate()
//  Invalidate all tlb enties
//  TODO: More detailed invalidation (by ASID)
.global tlb_invalidate
tlb_invalidate:
    dsb ishst
    tlbi vmalle1is
    dsb ish
    isb
    ret
