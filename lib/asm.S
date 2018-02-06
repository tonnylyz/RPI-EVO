.macro push_time_stack
    str x0, [sp, #-8]!
    ldr x0, =0xFFFFFF8001800000 // timestack top
    // save gpr
    stp x29, x30, [x0, #-16]!
    stp x27, x28, [x0, #-16]!
    stp x25, x26, [x0, #-16]!
    stp x23, x24, [x0, #-16]!
    stp x21, x22, [x0, #-16]!
    stp x19, x20, [x0, #-16]!
    stp x17, x18, [x0, #-16]!
    stp x15, x16, [x0, #-16]!
    stp x13, x14, [x0, #-16]!
    stp x11, x12, [x0, #-16]!
    stp x9, x10, [x0, #-16]!
    stp x7, x8, [x0, #-16]!
    stp x5, x6, [x0, #-16]!
    stp x3, x4, [x0, #-16]!
    stp x1, x2, [x0, #-16]!
    mrs x1, sp_el0 // fetch sp
    ldr x2, [sp], #8 // fetch x0 in stack
    stp x1, x2, [x0, #-16]!
    mrs x1, elr_el1 // fetch elr
    mrs x2, spsr_el1 // fetch spsr
    stp x1, x2, [x0, #-16]!
    // x0 = 0xFFFFFF80017FFEF0 (trapframe size: 0x110)
.endm

.macro pop_time_stack
    ldr x30, =0xFFFFFF80017FFEF0
    ldp x0, x1, [x30], #16
    msr spsr_el1, x0
    msr elr_el1, x1
    ldp x1, x0, [x30], #16
    msr sp_el0, x1
    ldp x1, x2, [x30], #16
    ldp x3, x4, [x30], #16
    ldp x5, x6, [x30], #16
    ldp x7, x8, [x30], #16
    ldp x9, x10, [x30], #16
    ldp x11, x12, [x30], #16
    ldp x13, x14, [x30], #16
    ldp x15, x16, [x30], #16
    ldp x17, x18, [x30], #16
    ldp x19, x20, [x30], #16
    ldp x21, x22, [x30], #16
    ldp x23, x24, [x30], #16
    ldp x25, x26, [x30], #16
    ldp x27, x28, [x30], #16
    ldr x29, [x30], #8
    ldr x30, [x30]
.endm

.macro vector handler
    .align 7
    b \handler
.endm

.align 11
.global vectors
vectors:
    vector el1_sync
    vector el1_irq
    vector el1_fiq
    vector el1_err

    vector el1_sync
    vector el1_irq
    vector el1_fiq
    vector el1_err

    vector el1_sync
    vector el1_irq
    vector el1_fiq
    vector el1_err

    vector el1_sync
    vector el1_irq
    vector el1_fiq
    vector el1_err

// Syscall and Pgfault
el1_sync:
    push_time_stack
    mrs x10, esr_el1
    lsr x10, x19, #26
    cmp x10, #0x24
    beq el1_sync_pgfault
    cmp x10, #0x15
    beq el1_sync_syscall
    bl handle_err
el1_sync_end:
    pop_time_stack
    eret

el1_sync_pgfault:
    ldr x0, =0xFFFFFF8000080000 // kernel stack
    mov sp, x0
    bl handle_pgfault
    b el1_sync_end

el1_sync_syscall:
    ldr x10, =0xFFFFFF8000080000 // kernel stack
    mov sp, x10
    bl handle_syscall
    b el1_sync_end

el1_fiq:
    ldr x0, =0xFFFFFF8000080000 // kernel stack
    mov sp, x0
    bl handle_int
    eret

el1_err:
    ldr x0, =0xFFFFFF8000080000 // kernel stack
    mov sp, x0
    bl handle_int
    eret

// Interupt (Clock)
el1_irq:
    push_time_stack
    ldr x0, =0xFFFFFF8000080000 // kernel stack
    mov sp, x0
    bl handle_int
    pop_time_stack
    eret