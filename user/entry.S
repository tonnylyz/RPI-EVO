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
    br x19
    nop