#include <printf.h>
#include <pmap.h>
#include <sd.h>
#include <env.h>
#include <kclock.h>
#include <sched.h>

u_char program[160 * 512];

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


    // fktest.elf
    //sd_readblock(3000, program, 151);
    //env_create(program, 76992);

    // pingpong.elf
    sd_readblock(4000, program, 151);
    env_create(program, 77152);

    kclock_init();
    printf("kclock_init ok!\n");

    while (1) {
        asm volatile ("nop");
    }
}
