#include <mmu.h>
#include <types.h>

typedef unsigned long Pde;
typedef unsigned long Pme;
typedef unsigned long Pte;

u_long freemem;

void boot_bzero(void *b, u_long len) {
    u_long max = (u_long) b + len;
    while ((u_long) b + 7 < max) {
        *(u_long *) b = 0;
        b += 8;
    }
    while ((u_long) b < max) {
        *(u_char *) b++ = 0;
    }
}

static void *boot_alloc(u_int n, u_int align, int clear) {
    u_long alloced_mem;
    freemem = ROUND(freemem, align);
    alloced_mem = freemem;
    freemem += n;
    if (clear) {
        boot_bzero((void *) alloced_mem, n);
    }
    return (void *) alloced_mem;
}

static Pte *boot_pgdir_walk(u_long *pgdir, u_long va) {
    // Use 39bit virtual address (4kB page and 3 levels page table)
    // [   9   |   9   |   9   |   12   ]
    //    Pde     Pme     Pte     Page
    Pte *pde;
    Pte *pme;
    Pte *pte;
    pde = (Pte *) (&pgdir[PDX(va)]);
    pme = (Pte *) (PTE_ADDR(*pde)) + PMX(va);
    if (!(*pde & PTE_4KB)) {
        pme = (Pte *) boot_alloc(BY2PG, BY2PG, 1);
        *pde = (u_long) pme | PTE_KERN | PTE_RW | PTE_AF | PTE_4KB;
        pme += PMX(va);
    }
    pte = (Pte *) (PTE_ADDR(*pme)) + PTX(va);
    if (!(*pme & PTE_4KB)) {
        pte = (Pte *) boot_alloc(BY2PG, BY2PG, 1);
        *pme = (u_long) pte | PTE_KERN | PTE_RW | PTE_AF | PTE_4KB;
        pte += PTX(va);
    }
    return pte;
}

void boot_map_segment(Pde *pgdir, u_long va, u_long size, u_long pa, int perm) {
    Pte *pte;
    int i;
    for (i = 0; i < size; i += BY2PG) {
        pte = boot_pgdir_walk(pgdir, va + i);
        *pte = PTE_ADDR(pa + i) | perm | PTE_KERN | PTE_RW | PTE_AF | PTE_4KB;
    }
}
/*

volatile unsigned int __attribute__((aligned(16))) mbox[36];

int mbox_call(unsigned char ch) {
#define MBOX_READ       ((volatile u_int*)(0x3F00B880))
#define MBOX_STATUS     ((volatile u_int*)(0x3F00B898))
#define MBOX_WRITE      ((volatile u_int*)(0x3F00B8A0))
    unsigned int r;
    do { asm volatile("nop"); } while (*MBOX_STATUS & 0x80000000);
    *MBOX_WRITE = (((unsigned int) ((unsigned long) &mbox) & ~0xF) | (ch & 0xF));
    while (1) {
        do { asm volatile("nop"); } while (*MBOX_STATUS & 0x40000000);
        r = *MBOX_READ;
        if ((unsigned char) (r & 0xF) == ch && (r & ~0xF) == (unsigned int) ((unsigned long) &mbox))
            return mbox[1] == 0x80000000;
    }
}
*/

void uart_init() {
#define UART0_IBRD      ((volatile u_int*)(0x3F201024))
#define UART0_FBRD      ((volatile u_int*)(0x3F201028))
#define UART0_LCRH      ((volatile u_int*)(0x3F20102C))
#define UART0_CR        ((volatile u_int*)(0x3F201030))
#define UART0_ICR       ((volatile u_int*)(0x3F201044))
#define GPFSEL1         ((volatile u_int*)(0x3F200004))
#define GPPUD           ((volatile u_int*)(0x3F200094))
#define GPPUDCLK0       ((volatile u_int*)(0x3F200098))
#define UART0_DR        ((volatile u_int*)(0x3F201000))
#define UART0_FR        ((volatile u_int*)(0x3F201018))
    register unsigned int r;
    *UART0_CR = 0;
/*    mbox[0] = 8 * 4;
    mbox[1] = 0;
    mbox[2] = 0x38002; // set clock rate
    mbox[3] = 12;
    mbox[4] = 8;
    mbox[5] = 2;           // UART clock
    mbox[6] = 4000000;     // 4Mhz
    mbox[7] = 0;
    mbox_call(8);*/
    r = *GPFSEL1;
    r &= ~((7 << 12) | (7 << 15)); // gpio14, gpio15
    r |= (4 << 12) | (4 << 15);    // alt0
    *GPFSEL1 = r;
    *GPPUD = 0;            // enable pins 14 and 15
    r = 150;
    while (r--) { asm volatile("nop"); }
    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    r = 150;
    while (r--) { asm volatile("nop"); }
    *GPPUDCLK0 = 0;        // flush GPIO setup
    *UART0_ICR = 0x7FF;    // clear interrupts
    *UART0_IBRD = 2;       // 115200 baud
    *UART0_FBRD = 0xB;
    *UART0_LCRH = 0b11 << 5; // 8n1
    *UART0_CR = 0x301;     // enable Tx, Rx, FIFO
}

void uart_send(unsigned int c) {
    do { asm volatile("nop"); } while (*UART0_FR & 0x20);
    *UART0_DR = c;
}

char uart_getc() {
    char r;
    do { asm volatile("nop"); } while (*UART0_FR & 0x10);
    r = (char) (*UART0_DR);
    return r == '\r' ? '\n' : r;
}

void uart_puts(char *s) {
    while (*s) {
        if (*s == '\n')
            uart_send('\r');
        uart_send(*s++);
    }
}

void uart_hex(unsigned int d) {
    unsigned int n;
    int c;
    for (c = 28; c >= 0; c -= 4) {
        // get highest tetrad
        n = (d >> c) & 0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n += n > 9 ? 0x37 : 0x30;
        uart_send(n);
    }
}

void vm_init() {
    uart_init();
    Pde *pgdir;
    freemem = KERNEL_PGDIR_PA;
    pgdir = boot_alloc(BY2PG, BY2PG, 1);

    // 0x00000000 - 0x3F000000 Normal memory
    boot_map_segment(pgdir, 0, MAXPA, 0, PTE_NORMAL | PTE_INNER_SHARE);
    // 0x3F000000 - 0x40000000 Device I/O
    boot_map_segment(pgdir, MAXPA, 0x01000000, MAXPA, PTE_DEVICE | PTE_OUTER_SHARE);
    // One more page for control Regs
    boot_map_segment(pgdir, 0x40000000, BY2PG, 0x40000000, PTE_DEVICE | PTE_OUTER_SHARE);

    uart_puts("uart_init ok!\n");
    uart_puts("boot_map_segment ok!\n");
}

