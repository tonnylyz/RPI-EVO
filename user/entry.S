.data
.global envs
envs: .word 0x80000000

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