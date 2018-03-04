#ifndef LIB_H
#define LIB_H

#define USER_LIB

#include <error.h>
#include <queue.h>
#include <trap.h>
#include <env.h>
#include <types.h>
#include <filesystem.h>

extern struct Env *envs;
extern struct Env *env;
extern struct Page *pages;
extern unsigned long *vpt, *vpm, *vpd;
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

void syscall_emmc_read(unsigned int sector, unsigned long va);

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

//////////////////////////////////////////////////////////////

void set_pgfault_handler(void (*fn)(unsigned long va));

int pageref(void *va);

/* File open modes */
#define	O_RDONLY	0x0000		/* open for reading only */
#define	O_WRONLY	0x0001		/* open for writing only */
#define	O_RDWR		0x0002		/* open for reading and writing */
#define	O_ACCMODE	0x0003		/* mask for above modes */

#define	O_CREAT		0x0100		/* create if nonexistent */
#define	O_TRUNC		0x0200		/* truncate to zero length */
#define	O_EXCL		0x0400		/* error if already exists */
#define O_MKDIR		0x0800		/* create directory, not regular file */

#define user_assert(x)	\
	do {	if (!(x)) user_panic("assertion failed: %s", #x); } while (0)

#endif
