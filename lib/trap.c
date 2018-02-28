#include <kclock.h>
#include <sched.h>
#include <printf.h>
#include <mmu.h>

u32 get_el() {
    u32 r;
    __asm__ __volatile__ (
    "mrs %0, currentel" : "=r"(r)
    );
    return r;
}

u32 get_esr() {
    u32 r;
    __asm__ __volatile__ (
    "mrs %0, esr_el1" : "=r"(r)
    );
    return r;
}

u64 get_far() {
    u64 r;
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

void handle_syscall(int no, char c) {
    printf("%c", c);
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