#ifndef _TRAP_H_
#define _TRAP_H_

struct Trapframe
{
    unsigned long spsr;
    unsigned long elr;
    unsigned long esr;
    unsigned long sp;
    unsigned long regs[31];
};

#endif
