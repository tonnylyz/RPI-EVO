#include <uart.h>

#define UART0_DR_KERN        ((volatile unsigned int*)(0xFFFFFF8000000000UL + 0x3F000000 + 0x00201000))
#define UART0_FR_KERN        ((volatile unsigned int*)(0xFFFFFF8000000000UL + 0x3F000000 + 0x00201018))

void uart_send_kern(unsigned int c) {
    do{asm volatile("nop");}while(*UART0_FR_KERN&0x20);
    *UART0_DR_KERN=c;
}

char uart_getc_kern() {
    char r;
    do{asm volatile("nop");}while(*UART0_FR_KERN&0x10);
    r=(char)(*UART0_DR_KERN);
    return r=='\r'?'\n':r;
}
