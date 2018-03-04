.data
.global envs
envs: .dword 0x80000000

.global __pgfault_handler
__pgfault_handler: .dword 0

.global vpt
vpt: .dword 0x7000000000UL
.global vpm
vpm: .dword 0x7040000000UL
.global vpd
vpd: .dword 0x7040200000UL

__sp_user: .dword 0
__elr_user: .dword 0

.extern libmain

.text
.global _start
_start:
	bl	libmain
	nop

.global msyscall
msyscall:
    svc #0
    ret

.global __asm_pgfault_handler
__asm_pgfault_handler:
    ldr x19, __pgfault_handler
    blr x19
    nop
    ldr x0, [sp, #8]
    adr x1, __elr_user
    str x0, [x1]
    ldr x0, [sp, #24]
    adr x1, __sp_user
    str x0, [x1]

    ldr x0, [sp, #32]
    ldr x1, [sp, #40]
    ldr x2, [sp, #48]
    ldr x3, [sp, #56]
    ldr x4, [sp, #64]
    ldr x5, [sp, #72]
    ldr x6, [sp, #80]
    ldr x7, [sp, #88]

    ldr x8,  [sp, #96]
    ldr x9,  [sp, #104]
    ldr x10, [sp, #112]
    ldr x11, [sp, #120]
    ldr x12, [sp, #128]
    ldr x13, [sp, #136]
    ldr x14, [sp, #144]
    ldr x15, [sp, #152]

    ldr x16, [sp, #160]
    ldr x17, [sp, #168]
    ldr x18, [sp, #176] // x18 PR (reserved purpose)
    ldr x19, [sp, #184]
    ldr x20, [sp, #192]
    ldr x21, [sp, #200]
    ldr x22, [sp, #208]
    ldr x23, [sp, #216]

    ldr x24, [sp, #224]
    ldr x25, [sp, #232]
    ldr x26, [sp, #240]
    ldr x27, [sp, #248]
    ldr x28, [sp, #256]
    ldr x29, [sp, #264]
    ldr x30, [sp, #272]

    adr x18, __sp_user
    ldr x18, [x18]
    mov sp, x18
    adr x18, __elr_user
    ldr x18, [x18]
    br x18
