#include <timer.h>
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
    printf("\n[Clock Interrupt]\n");
    clear_clock_int();
    sched_yield();
    setup_clock_int(0);
}

void handle_syscall(char c) {
    printf("%c", c);
}

void handle_pgfault() {
    printf("\n[System Exception]\n");
    printf("Page fault : va : [%l016x]\n", get_far());
    printf("esr : [%08x]\n", get_esr());
    while (1) {
        asm volatile ("nop");
    }
}

void handle_err() {
    printf("\n[System Exception]\n");
    printf("Kernel died\n");
    printf("esr : [%08x]\n", get_esr());
    printf("far : [%l016x]\n", get_far());
    while (1) {
        asm volatile ("nop");
    }
}