#include <kclock.h>

#define ARCH_TIMER_CTRL_ENABLE		(1 << 0)
#define ARCH_TIMER_CTRL_IT_MASK		(1 << 1)
#define ARCH_TIMER_CTRL_IT_STAT		(1 << 2)

#define TIMER_IRQ_CTL ((volatile unsigned int*)(0x40000040 | 0xFFFFFF8000000000UL))

void kclock_next(unsigned long evt) {
    if (evt == 0) evt = 10000000;
    register unsigned long ctl;
    asm volatile ("mrs %0, cntp_ctl_el0" : "=r"(ctl));
    ctl |= ARCH_TIMER_CTRL_ENABLE;
    ctl &= ~ARCH_TIMER_CTRL_IT_MASK;
    asm volatile ("msr cntp_tval_el0, %0" :: "r" (evt));
    asm volatile ("msr cntp_ctl_el0, %0" :: "r" (ctl));
}

void kclock_init() {
    *TIMER_IRQ_CTL = 0b1111;
    kclock_next(0);
}
