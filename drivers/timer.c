#include <timer.h>

#define TIMER_INT  ((volatile unsigned int*)(0x3F00B210 | 0xFFFFFF8000000000UL))
#define TIMER_BASE ((volatile unsigned int*)(0x3F003000 | 0xFFFFFF8000000000UL))
#define TIMER_CLO  ((volatile unsigned int*)(0x3F003004 | 0xFFFFFF8000000000UL))
#define TIMER_C3   ((volatile unsigned int*)(0x3F003018 | 0xFFFFFF8000000000UL))

void setup_clock_int(unsigned int ns) {
    static unsigned int last;
    if (ns != 0) {
        last = ns;
    }
    *TIMER_INT = 1 << 3;
    *TIMER_C3 = last + *TIMER_CLO;
}

void clear_clock_int() {
    *TIMER_BASE = 1 << 3;
}