#include <lib.h>

extern unsigned long msyscall(int no, unsigned long a1, unsigned long a2, unsigned long a3, unsigned long a4, unsigned long a5);

void syscall_putchar(char c) {
    msyscall(0, c, 0, 0, 0, 0);
}

unsigned int syscall_getenvid() {
    return (unsigned int)msyscall(1, 0, 0, 0, 0, 0);
}

void syscall_yield() {
    msyscall(2, 0, 0, 0, 0, 0);
}

int syscall_env_destroy(unsigned int envid) {
    return (int)msyscall(3, envid, 0, 0, 0, 0);
}

int syscall_set_pgfault_handler(unsigned int envid, void (*func)(void), unsigned long xstacktop) {
    return (int)msyscall(4, envid, (unsigned long)func, xstacktop, 0, 0);
}

int syscall_mem_alloc(unsigned int envid, unsigned long va, unsigned long perm) {
    return (int)msyscall(5, envid, va, perm, 0, 0);
}

int syscall_mem_map(unsigned int srcid, unsigned long srcva, unsigned int dstid, unsigned long dstva, unsigned long perm) {
    return (int)msyscall(6, srcid, srcva, dstid, dstva, perm);
}

int syscall_mem_unmap(unsigned int envid, unsigned long va) {
    return (int)msyscall(7, envid, va, 0, 0, 0);
}

unsigned int syscall_env_alloc() {
    return (unsigned int)msyscall(8, 0, 0, 0, 0, 0);
}

int syscall_set_env_status(unsigned int envid, unsigned int status) {
    return (int)msyscall(9, envid, status, 0, 0, 0);
}

int syscall_set_trapframe(unsigned int envid, struct Trapframe *tf) {
    return (int)msyscall(10, envid, (unsigned long)tf, 0, 0, 0);
}

void syscall_panic(char *msg) {
    msyscall(11, (unsigned long)msg, 0, 0, 0, 0);
}

int syscall_ipc_can_send(unsigned int envid, unsigned long value, unsigned long srcva, unsigned long perm) {
    return (int)msyscall(12, envid, value, srcva, perm, 0);
}

void syscall_ipc_recv(unsigned long dstva) {
    msyscall(13, dstva, 0, 0, 0, 0);
}

char syscall_cgetc() {
    return (char)msyscall(14, 0, 0, 0, 0, 0);
}
