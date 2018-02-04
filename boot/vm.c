#include <mmu.h>
u_long freemem;

void boot_bzero(void *b, size_t len) {
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


#define MMIO_BASE       0x3F000000

#define GPFSEL0         ((volatile unsigned int*)(MMIO_BASE+0x00200000))
#define GPFSEL1         ((volatile unsigned int*)(MMIO_BASE+0x00200004))
#define GPFSEL2         ((volatile unsigned int*)(MMIO_BASE+0x00200008))
#define GPFSEL3         ((volatile unsigned int*)(MMIO_BASE+0x0020000C))
#define GPFSEL4         ((volatile unsigned int*)(MMIO_BASE+0x00200010))
#define GPFSEL5         ((volatile unsigned int*)(MMIO_BASE+0x00200014))
#define GPSET0          ((volatile unsigned int*)(MMIO_BASE+0x0020001C))
#define GPSET1          ((volatile unsigned int*)(MMIO_BASE+0x00200020))
#define GPCLR0          ((volatile unsigned int*)(MMIO_BASE+0x00200028))
#define GPLEV0          ((volatile unsigned int*)(MMIO_BASE+0x00200034))
#define GPLEV1          ((volatile unsigned int*)(MMIO_BASE+0x00200038))
#define GPEDS0          ((volatile unsigned int*)(MMIO_BASE+0x00200040))
#define GPEDS1          ((volatile unsigned int*)(MMIO_BASE+0x00200044))
#define GPHEN0          ((volatile unsigned int*)(MMIO_BASE+0x00200064))
#define GPHEN1          ((volatile unsigned int*)(MMIO_BASE+0x00200068))
#define GPPUD           ((volatile unsigned int*)(MMIO_BASE+0x00200094))
#define GPPUDCLK0       ((volatile unsigned int*)(MMIO_BASE+0x00200098))
#define GPPUDCLK1       ((volatile unsigned int*)(MMIO_BASE+0x0020009C))

#define UART0_DR        ((volatile unsigned int*)(MMIO_BASE+0x00201000))
#define UART0_FR        ((volatile unsigned int*)(MMIO_BASE+0x00201018))
#define UART0_IBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201024))
#define UART0_FBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201028))
#define UART0_LCRH      ((volatile unsigned int*)(MMIO_BASE+0x0020102C))
#define UART0_CR        ((volatile unsigned int*)(MMIO_BASE+0x00201030))
#define UART0_IMSC      ((volatile unsigned int*)(MMIO_BASE+0x00201038))
#define UART0_ICR       ((volatile unsigned int*)(MMIO_BASE+0x00201044))


/* mailbox message buffer */
volatile unsigned int  __attribute__((aligned(16))) mbox[36];

#define VIDEOCORE_MBOX  (MMIO_BASE+0x0000B880)
#define MBOX_READ       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x0))
#define MBOX_POLL       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x10))
#define MBOX_SENDER     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x14))
#define MBOX_STATUS     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x18))
#define MBOX_CONFIG     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x1C))
#define MBOX_WRITE      ((volatile unsigned int*)(VIDEOCORE_MBOX+0x20))
#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000

#define MBOX_REQUEST    0

/* channels */
#define MBOX_CH_POWER   0
#define MBOX_CH_FB      1
#define MBOX_CH_VUART   2
#define MBOX_CH_VCHIQ   3
#define MBOX_CH_LEDS    4
#define MBOX_CH_BTNS    5
#define MBOX_CH_TOUCH   6
#define MBOX_CH_COUNT   7
#define MBOX_CH_PROP    8

/* tags */
#define MBOX_TAG_GETSERIAL      0x10004
#define MBOX_TAG_SETCLKRATE     0x38002
#define MBOX_TAG_LAST           0

int mbox_call(unsigned char ch)
{
    unsigned int r;
    /* wait until we can write to the mailbox */
    do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    *MBOX_WRITE = (((unsigned int)((unsigned long)&mbox)&~0xF) | (ch&0xF));
    /* now wait for the response */
    while(1) {
        /* is there a response? */
        do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_EMPTY);
        r=*MBOX_READ;
        /* is it a response to our message? */
        if((unsigned char)(r&0xF)==ch && (r&~0xF)==(unsigned int)((unsigned long)&mbox))
            /* is it a valid successful response? */
            return mbox[1]==MBOX_RESPONSE;
    }
    return 0;
}

void uart_init()
{
    register unsigned int r;
    *UART0_CR = 0;
    mbox[0] = 8*4;
    mbox[1] = MBOX_REQUEST;
    mbox[2] = MBOX_TAG_SETCLKRATE; // set clock rate
    mbox[3] = 12;
    mbox[4] = 8;
    mbox[5] = 2;           // UART clock
    mbox[6] = 4000000;     // 4Mhz
    mbox[7] = MBOX_TAG_LAST;
    mbox_call(MBOX_CH_PROP);

    r=*GPFSEL1;
    r&=~((7<<12)|(7<<15)); // gpio14, gpio15
    r|=(4<<12)|(4<<15);    // alt0
    *GPFSEL1 = r;
    *GPPUD = 0;            // enable pins 14 and 15
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = (1<<14)|(1<<15);
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = 0;        // flush GPIO setup

    *UART0_ICR = 0x7FF;    // clear interrupts
    *UART0_IBRD = 2;       // 115200 baud
    *UART0_FBRD = 0xB;
    *UART0_LCRH = 0b11<<5; // 8n1
    *UART0_CR = 0x301;     // enable Tx, Rx, FIFO
}

void uart_send(unsigned int c) {
    do{asm volatile("nop");}while(*UART0_FR&0x20);
    *UART0_DR=c;
}

char uart_getc() {
    char r;
    do{asm volatile("nop");}while(*UART0_FR&0x10);
    r=(char)(*UART0_DR);
    return r=='\r'?'\n':r;
}

void uart_puts(char *s) {
    while(*s) {
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s++);
    }
}

void uart_hex(unsigned int d) {
    unsigned int n;
    int c;
    for(c=28;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
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
    
    uart_puts("uart_init ok!\n");
    uart_puts("boot_map_segment ok!\n");
}

