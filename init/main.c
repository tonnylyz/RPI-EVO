#include <printf.h>
#include <pmap.h>
#include <uart.h>

void main() {
    printf("mmu started!\n");
    page_init();
    printf("page_init ok!\n");
    while (1) {
        char c = uart_getc_kern();
        uart_send_kern(c);
    }
}
