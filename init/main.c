#include <printf.h>
#include <drivers/include/uart.h>

void main() {
    uart_init();
    printf("System started!\n");
    while (1) {
        char c = uart_getc();
        uart_send(c);
    }
}
