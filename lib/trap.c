#include <kclock.h>
#include <sched.h>
#include <printf.h>
#include <mmu.h>
#include <syscall_all.h>
#include <types.h>

u_int get_el() {
    u_int r;
    __asm__ __volatile__ (
    "mrs %0, currentel" : "=r"(r)
    );
    return r;
}

u_int get_esr() {
    u_int r;
    __asm__ __volatile__ (
    "mrs %0, esr_el1" : "=r"(r)
    );
    return r;
}

u_long get_far() {
    u_long r;
    __asm__ __volatile__ (
    "mrs %0, far_el1" : "=r"(r)
    );
    return r;
}

// Handle Clock Interrupt
void handle_int() {
    sched_yield();
    kclock_next(0);
}

void handle_syscall(int no, u_long a1, u_long a2, u_long a3, u_long a4, u_long a5) {
    u_long result;
    switch (no) {
        default:
        case 0:
            sys_putchar(no, (char) a1);
            break;
        case 1:
            result = sys_getenvid();
            sys_set_return(result);
            break;
        case 2:
            sys_yield();
            break;
        case 3:
            result = (u_long) sys_env_destroy(no, (u_int) a1);
            sys_set_return(result);
            break;
        case 4:
            result = (u_long) sys_set_pgfault_handler(no, (u_int) a1, a2, a3);
            sys_set_return(result);
            break;
        case 5:
            result = (u_long) sys_mem_alloc(no, (u_int) a1, a2, a3);
            sys_set_return(result);
            break;
        case 6:
            result = (u_long) sys_mem_map(no, (u_int) a1, a2, (u_int) a3, a4, a5);
            sys_set_return(result);
            break;
        case 7:
            result = (u_long) sys_mem_unmap(no, (u_int) a1, a2);
            sys_set_return(result);
            break;
        case 8:
            result = (u_long) sys_env_alloc();
            sys_set_return(result);
            break;
        case 9:
            result = (u_long) sys_set_env_status(no, (u_int) a1, (u_int) a2);
            sys_set_return(result);
            break;
        case 10:
            result = (u_long) sys_set_trapframe(no, (u_int) a1, (struct Trapframe *) a2);
            sys_set_return(result);
            break;
        case 11:
            sys_panic(no, (char *) a1);
            break;
        case 12:
            result = (u_long) sys_ipc_can_send(no, (u_int) a1, a2, a3, a4);
            sys_set_return(result);
            break;
        case 13:
            sys_ipc_recv(no, a1);
            break;
        case 14:
            result = (u_long) sys_cgetc();
            sys_set_return(result);
            break;
        case 15:
            result = sys_pgtable_entry(no, a1);
            sys_set_return(result);
            break;
        case 16:
            result = sys_fork();
            sys_set_return(result);
            break;
    }
}

void handle_pgfault() {
    printf("\n[Page fault]\n");
    printf("esr : [%08x]\n", get_esr());
    printf("va  : [%l016x]\n", get_far());
    while (1) {
        asm volatile ("nop");
    }
}

void handle_sync() {
    printf("\n[Sync Exception]\n");
    printf("esr : [%08x]\n", get_esr());
    printf("far : [%l016x]\n", get_far());
    while (1) {
        asm volatile ("nop");
    }
}

void handle_err() {
    printf("\n[Err Exception]\n");
    printf("esr : [%08x]\n", get_esr());
    printf("far : [%l016x]\n", get_far());
    while (1) {
        asm volatile ("nop");
    }
}

void handle_fiq() {
    printf("\n[FIQ Interrupt]\n");
    printf("esr : [%08x]\n", get_esr());
    printf("far : [%l016x]\n", get_far());
    while (1) {
        asm volatile ("nop");
    }
}