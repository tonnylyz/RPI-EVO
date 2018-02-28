.equ TIMESTACK_TOP, 0xFFFFFF8001800000
.equ TIMESTACK_BTM, 0xFFFFFF80017FFEE8
.equ KERNEL_STACK_TOP, 0xFFFFFF8000080000

.macro push_time_stack
    str x0, [sp, #-16]!
    ldr x0, =TIMESTACK_TOP
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
    mov x21, x0
    ldr x0, [sp], #16
    mrs x20, sp_el0
    stp x20, x0, [x21, #-16]!
    mrs x19, esr_el1
    mrs x20, elr_el1
    stp x20, x19, [x21, #-16]!
    mrs x19, spsr_el1
    str x19, [x21, #-8]!
.endm

.macro pop_time_stack
    ldr x30, =TIMESTACK_BTM
    ldr x0, [x30], #8
    msr spsr_el1, x0
    ldp x0, x1, [x30], #16
    msr elr_el1, x0
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
    ldp x29, x30, [x30] // do NOT add `#0` here which makes it an invalid instruction
.endm

.macro vector handler
    .align 7
    b \handler
.endm

// Exception Vector Table
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
    mrs x19, esr_el1
    lsr x19, x19, #26
    cmp x19, #0x24
    beq el1_sync_pgfault
    cmp x19, #0x15
    beq el1_sync_syscall
    bl handle_err
el1_sync_end:
    pop_time_stack
    eret

el1_sync_pgfault:
    ldr x19, =KERNEL_STACK_TOP
    mov sp, x19
    bl handle_pgfault
    b el1_sync_end

el1_sync_syscall:
    ldr x19, =KERNEL_STACK_TOP
    mov sp, x19
    bl handle_syscall
    b el1_sync_end

el1_fiq:
    ldr x19, =KERNEL_STACK_TOP
    mov sp, x19
    bl handle_fiq
    eret

el1_err:
    ldr x19, =KERNEL_STACK_TOP
    mov sp, x19
    bl handle_err
    eret

// Interupt (Clock)
el1_irq:
    push_time_stack
    ldr x19, =KERNEL_STACK_TOP
    mov sp, x19
    bl handle_int
    pop_time_stack
    eret