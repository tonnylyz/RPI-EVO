.data
.global envs
envs: .word 0x80000000
.dword 0 // interesting bug?
.global __pgfault_handler
__pgfault_handler: .word 0

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