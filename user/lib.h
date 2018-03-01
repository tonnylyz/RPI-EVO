#ifndef LIB_H
#define LIB_H

#define USER_LIB

#include <error.h>
#include <queue.h>
#include <trap.h>
#include <env.h>

extern struct Env *envs;

/////////////////////////////////////////////////////////////
//                          Printf                         //
/////////////////////////////////////////////////////////////
#include <stdarg.h>

void user_lp_Print(void (*output)(void *, const char *, int), void *arg, const char *fmt, va_list ap);

void writef(char *fmt, ...);

void _user_panic(const char *, int, const char *, ...)
__attribute__((noreturn));

#define user_panic(...) _user_panic(__FILE__, __LINE__, __VA_ARGS__)

/////////////////////////////////////////////////////////////
//                          Fork                           //
/////////////////////////////////////////////////////////////
int fork();

/////////////////////////////////////////////////////////////
//                       Syscall                           //
/////////////////////////////////////////////////////////////
void syscall_putchar(char c);

unsigned int syscall_getenvid();

void syscall_yield();

int syscall_env_destroy(unsigned int envid);

int syscall_set_pgfault_handler(unsigned int envid, void (*func)(void), unsigned long xstacktop);

int syscall_mem_alloc(unsigned int envid, unsigned long va, unsigned long perm);

int syscall_mem_map(unsigned int srcid, unsigned long srcva, unsigned int dstid, unsigned long dstva, unsigned long perm);

int syscall_mem_unmap(unsigned int envid, unsigned long va);

unsigned int syscall_env_alloc();

int syscall_set_env_status(unsigned int envid, unsigned int status);

int syscall_set_trapframe(unsigned int envid, struct Trapframe *tf);

void syscall_panic(char *msg);

void syscall_ipc_recv(unsigned long dstva);

int syscall_ipc_can_send(unsigned int envid, unsigned long value, unsigned long srcva, unsigned long perm);

char syscall_cgetc();

unsigned long syscall_pgtable_entry(unsigned long va);

unsigned int syscall_fork();

/////////////////////////////////////////////////////////////
//                         String                          //
/////////////////////////////////////////////////////////////
int strlen(const char *s);

char *strcpy(char *dst, const char *src);

const char *strchr(const char *s, char c);

void *memcpy(void *destaddr, void const *srcaddr, unsigned long len);

int strcmp(const char *p, const char *q);

void user_bcopy(const void *src, void *dst, unsigned long len);

void user_bzero(void *b, unsigned long len);

/////////////////////////////////////////////////////////////
//                          IPC                            //
/////////////////////////////////////////////////////////////
void ipc_send(unsigned int whom, unsigned long val, unsigned long srcva, unsigned long perm);

unsigned long ipc_recv(unsigned int *whom, unsigned long dstva, unsigned long *perm);

#endif
