#ifndef RPI_LAB_SYSCALL_ALL_H
#define RPI_LAB_SYSCALL_ALL_H

#include <mmu.h>

struct Trapframe;

void sys_set_return(unsigned long r);

void sys_putchar(int sysno, char c);

unsigned int sys_getenvid();

void sys_yield();

int sys_env_destroy(int sysno, unsigned int envid);

int sys_set_pgfault_handler(int sysno, unsigned int envid, unsigned long func, unsigned long xstacktop);

int sys_mem_alloc(int sysno, unsigned int envid, unsigned long va, unsigned long perm);

int sys_mem_map(int sysno, unsigned int srcid, unsigned long srcva, unsigned int dstid, unsigned long dstva, unsigned long perm);

int sys_mem_unmap(int sysno, unsigned int envid, unsigned long va);

unsigned int sys_env_alloc();

int sys_set_env_status(int sysno, unsigned int envid, unsigned int status);

void sys_set_trapframe(int sysno, unsigned int envid, struct Trapframe *tf);

void sys_panic(int sysno, char *msg);

void sys_ipc_recv(int sysno, unsigned long dstva);

int sys_ipc_can_send(int sysno, unsigned int envid, unsigned long value, unsigned long srcva, unsigned long perm);

char sys_cgetc();

unsigned long sys_pgtable_entry(int sysno, unsigned long va);

#endif //RPI_LAB_SYSCALL_ALL_H
