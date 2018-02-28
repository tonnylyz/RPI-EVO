#include <printf.h>
#include <pmap.h>
#include <sd.h>
#include <env.h>
#include <kclock.h>
#include <sched.h>

u_char program[147 * 512]; // 75000 byte

void main() {
    int r;
    printf("mmu started!\n");
    page_init();
    printf("page_init ok!\n");
    r = sd_init();
    if (r == 0) {
        printf("sd_init ok!\n");
    } else {
        printf("sd_init failed!\n");
    }
    env_init();
    printf("env_init ok!\n");


    sd_readblock(1000, program, 147);
    printf("%8x\n", ((int *)(program))[0]);
    env_create(program, 75000);

    sd_readblock(10000, program, 147);
    printf("%8x\n", ((int *)(program))[0]);
    env_create(program, 75000);

    
    kclock_init();
    printf("kclock_init ok!\n");

    sched_yield();

    while (1) {
        asm volatile ("nop");
    }
}
