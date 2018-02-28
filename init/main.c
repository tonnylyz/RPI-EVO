#include <printf.h>
#include <pmap.h>
#include <sd.h>
#include <env.h>
#include <kclock.h>
#include <sched.h>

u_char program[130 * 512];

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


    sd_readblock(1000, program, 130);
    printf("%8x\n", ((int *)(program))[0]);
    env_create(program, 66560);

    sd_readblock(2000, program, 130);
    printf("%8x\n", ((int *)(program))[0]);
    env_create(program, 66560);

    
    kclock_init();
    printf("kclock_init ok!\n");

    sched_yield();

    while (1) {
        asm volatile ("nop");
    }
}
