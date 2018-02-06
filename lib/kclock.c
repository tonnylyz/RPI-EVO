#include <kclock.h>
#include <timer.h>

void kclock_init() {
    setup_clock_int(100000);
}

