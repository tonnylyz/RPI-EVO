#include <sd.h>
#include <mmu.h>

#define GPPUD               ((volatile u_int*)(0x3F200094 | 0xFFFFFF8000000000UL))
#define EMMC_STATUS         ((volatile u_int*)(0x3F300024 | 0xFFFFFF8000000000UL))
#define EMMC_INTERRUPT      ((volatile u_int*)(0x3F300030 | 0xFFFFFF8000000000UL))
#define EMMC_BLKSIZECNT     ((volatile u_int*)(0x3F300004 | 0xFFFFFF8000000000UL))
#define EMMC_ARG1           ((volatile u_int*)(0x3F300008 | 0xFFFFFF8000000000UL))
#define EMMC_CMDTM          ((volatile u_int*)(0x3F30000C | 0xFFFFFF8000000000UL))
#define EMMC_RESP0          ((volatile u_int*)(0x3F300010 | 0xFFFFFF8000000000UL))
#define EMMC_RESP1          ((volatile u_int*)(0x3F300014 | 0xFFFFFF8000000000UL))
#define EMMC_RESP2          ((volatile u_int*)(0x3F300018 | 0xFFFFFF8000000000UL))
#define EMMC_RESP3          ((volatile u_int*)(0x3F30001C | 0xFFFFFF8000000000UL))
#define EMMC_DATA           ((volatile u_int*)(0x3F300020 | 0xFFFFFF8000000000UL))
#define EMMC_CONTROL0       ((volatile u_int*)(0x3F300028 | 0xFFFFFF8000000000UL))
#define EMMC_CONTROL1       ((volatile u_int*)(0x3F30002C | 0xFFFFFF8000000000UL))
#define EMMC_INT_MASK       ((volatile u_int*)(0x3F300034 | 0xFFFFFF8000000000UL))
#define EMMC_INT_EN         ((volatile u_int*)(0x3F300038 | 0xFFFFFF8000000000UL))
#define EMMC_SLOTISR_VER    ((volatile u_int*)(0x3F3000FC | 0xFFFFFF8000000000UL))
unsigned long sd_scr[2], sd_rca, sd_err, sd_hv;

void wait_cycles(unsigned int n) {
    if (n) while (n--) { asm volatile("nop"); }
}

void wait_msec(unsigned int n) {
    register unsigned long f, t, r;
    asm volatile ("mrs %0, cntfrq_el0" : "=r"(f));
    asm volatile ("mrs %0, cntpct_el0" : "=r"(t));
    t += ((f / 1000) * n) / 1000;
    do { asm volatile ("mrs %0, cntpct_el0" : "=r"(r)); } while (r < t);
}

int sd_status(unsigned int mask) {
    int cnt = 500000;
    while ((*EMMC_STATUS & mask) && !(*EMMC_INTERRUPT & 0x017E8000) && cnt--) wait_msec(1);
    return (cnt <= 0 || (*EMMC_INTERRUPT & 0x017E8000)) ? -2 : 0;
}

int sd_int(unsigned int mask) {
    unsigned int r, m = mask | 0x017E8000;
    int cnt = 1000000;
    while (!(*EMMC_INTERRUPT & m) && cnt--) wait_msec(1);
    r = *EMMC_INTERRUPT;
    if (cnt <= 0 || (r & 0x00010000) || (r & 0x00100000)) {
        *EMMC_INTERRUPT = r;
        return -1;
    }
    else if (r & 0x017E8000) {
        *EMMC_INTERRUPT = r;
        return -2;
    }
    *EMMC_INTERRUPT = mask;
    return 0;
}

int sd_cmd(unsigned int code, unsigned int arg) {
    int r = 0;
    sd_err = 0;
    if (code & 0x80000000) {
        r = sd_cmd(0x37000000 | (sd_rca ? 0x00020000 : 0), sd_rca);
        if (sd_rca && !r) {
            sd_err = -2;
            return 0;
        }
        code &= ~0x80000000;
    }
    if (sd_status(0x00000001)) {
        sd_err = -1;
        return 0;
    }
    *EMMC_INTERRUPT = *EMMC_INTERRUPT;
    *EMMC_ARG1 = arg;
    *EMMC_CMDTM = code;
    if (code == (0x29020000 | 0x80000000)) wait_msec(1000);
    else if (code == 0x08020000 || code == 0x37000000)
        wait_msec(100);
    if ((r = sd_int(0x00000001))) {
        sd_err = r;
        return 0;
    }
    r = *EMMC_RESP0;
    if (code == 0x00000000 || code == 0x37000000) return 0;
    else if (code == (0x37000000 | 0x00020000)) return r & 0x00000020;
    else if (code == (0x29020000 | 0x80000000)) return r;
    else if (code == 0x08020000) return r == arg ? 0 : -2;
    else if (code == 0x02010000) {
        r |= *EMMC_RESP3;
        r |= *EMMC_RESP2;
        r |= *EMMC_RESP1;
        return r;
    }
    else if (code == 0x03020000) {
        sd_err = (((r & 0x1fff)) | ((r & 0x2000) << 6) | ((r & 0x4000) << 8) | ((r & 0x8000) << 8)) & 0xfff9c004;
        return r & 0xffff0000;
    }
    return r & 0xfff9c004;
}

int sd_clk(unsigned int f) {
    unsigned int d, c = 41666666 / f, x, s = 32, h = 0;
    int cnt = 100000;
    while ((*EMMC_STATUS & (0x00000001 | 0x00000002)) && cnt--) wait_msec(1);
    if (cnt <= 0) {
        return -2;
    }
    *EMMC_CONTROL1 &= ~0x00000004;
    wait_msec(10);
    x = c - 1;
    if (!x) s = 0;
    else {
        if (!(x & 0xffff0000u)) {
            x <<= 16;
            s -= 16;
        }
        if (!(x & 0xff000000u)) {
            x <<= 8;
            s -= 8;
        }
        if (!(x & 0xf0000000u)) {
            x <<= 4;
            s -= 4;
        }
        if (!(x & 0xc0000000u)) {
            x <<= 2;
            s -= 2;
        }
        if (!(x & 0x80000000u)) {
            x <<= 1;
            s -= 1;
        }
        if (s > 0) s--;
        if (s > 7) s = 7;
    }
    if (sd_hv > 1) d = c; else d = (1 << s);
    if (d <= 2) {
        d = 2;
        s = 0;
    }
    if (sd_hv > 1) h = (d & 0x300) >> 2;
    d = (((d & 0x0ff) << 8) | h);
    *EMMC_CONTROL1 = (*EMMC_CONTROL1 & 0xffff003f) | d;
    wait_msec(10);
    *EMMC_CONTROL1 |= 0x00000004;
    wait_msec(10);
    cnt = 10000;
    while (!(*EMMC_CONTROL1 & 0x00000002) && cnt--) wait_msec(10);
    if (cnt <= 0) {
        return -2;
    }
    return 0;
}

int sd_init() {
#define GPFSEL4         ((volatile unsigned int*)(0x3F000000+0x00200010))
#define GPFSEL5         ((volatile unsigned int*)(0x3F000000+0x00200014))
#define GPHEN1          ((volatile unsigned int*)(0x3F000000+0x00200068))
#define GPPUDCLK1       ((volatile unsigned int*)(0x3F000000+0x0020009C))
    long r, cnt, ccs = 0;
    // GPIO_CD
    r = *GPFSEL4;
    r &= ~(7 << (7 * 3));
    *GPFSEL4 = r;
    *GPPUD = 2;
    wait_cycles(150);
    *GPPUDCLK1 = (1 << 15);
    wait_cycles(150);
    *GPPUD = 0;
    *GPPUDCLK1 = 0;
    r = *GPHEN1;
    r |= 1 << 15;
    *GPHEN1 = r;

    // GPIO_CLK, GPIO_CMD
    r = *GPFSEL4;
    r |= (7 << (8 * 3)) | (7 << (9 * 3));
    *GPFSEL4 = r;
    *GPPUD = 2;
    wait_cycles(150);
    *GPPUDCLK1 = (1 << 16) | (1 << 17);
    wait_cycles(150);
    *GPPUD = 0;
    *GPPUDCLK1 = 0;

    // GPIO_DAT0, GPIO_DAT1, GPIO_DAT2, GPIO_DAT3
    r = *GPFSEL5;
    r |= (7 << (0 * 3)) | (7 << (1 * 3)) | (7 << (2 * 3)) | (7 << (3 * 3));
    *GPFSEL5 = r;
    *GPPUD = 2;
    wait_cycles(150);
    *GPPUDCLK1 = (1 << 18) | (1 << 19) | (1 << 20) | (1 << 21);
    wait_cycles(150);
    *GPPUD = 0;
    *GPPUDCLK1 = 0;

    sd_hv = (*EMMC_SLOTISR_VER & 0x00ff0000) >> 16;
    // Reset the card.
    *EMMC_CONTROL0 = 0;
    *EMMC_CONTROL1 |= 0x01000000;
    cnt = 10000;
    do { wait_msec(10); } while ((*EMMC_CONTROL1 & 0x01000000) && cnt--);
    if (cnt <= 0) {
        return -2;
    }
    *EMMC_CONTROL1 |= 0x00000001 | 0x000e0000;
    wait_msec(10);
    // Set clock to setup frequency.
    if ((r = sd_clk(400000))) return r;
    *EMMC_INT_EN = 0xffffffff;
    *EMMC_INT_MASK = 0xffffffff;
    sd_scr[0] = sd_scr[1] = sd_rca = sd_err = 0;
    sd_cmd(0x00000000, 0);
    if (sd_err) return sd_err;

    sd_cmd(0x08020000, 0x000001AA);
    if (sd_err) return sd_err;
    cnt = 6;
    r = 0;
    while (!(r & 0x80000000) && cnt--) {
        wait_cycles(400);
        r = sd_cmd((0x29020000 | 0x80000000), 0x51ff8000);
        if (sd_err != -1 && sd_err != 0) {
            return sd_err;
        }
    }
    if (!(r & 0x80000000) || !cnt) return -1;
    if (!(r & 0x00ff8000)) return -2;
    if (r & 0x40000000) ccs = 0x00000001;

    sd_cmd(0x02010000, 0);

    sd_rca = sd_cmd(0x03020000, 0);
    if (sd_err) return sd_err;

    if ((r = sd_clk(25000000))) return r;

    sd_cmd(0x07030000, sd_rca);
    if (sd_err) return sd_err;

    if (sd_status(0x00000002)) return -1;
    *EMMC_BLKSIZECNT = (1 << 16) | 8;
    sd_cmd((0x33220010 | 0x80000000), 0);
    if (sd_err) return sd_err;
    if (sd_int(0x00000020)) return -1;

    r = 0;
    cnt = 100000;
    while (r < 2 && cnt) {
        if (*EMMC_STATUS & 0x00000800)
            sd_scr[r++] = *EMMC_DATA;
        else
            wait_msec(1);
    }
    if (r != 2) return -1;
    if (sd_scr[0] & 0x00000400) {
        sd_cmd((0x06020000 | 0x80000000), sd_rca | 2);
        if (sd_err) return sd_err;
        *EMMC_CONTROL0 |= 0x00000002;
    }
    sd_scr[0] &= ~0x00000001;
    sd_scr[0] |= ccs;
    return 0;
}

int sd_readblock(unsigned int lba, unsigned char *buffer, unsigned int num) {
    int r, c = 0, d;
    if (num < 1) num = 1;
    if (sd_status(0x00000002)) {
        sd_err = -1;
        return 0;
    }
    unsigned int *buf = (unsigned int *) buffer;
    if (sd_scr[0] & 0x00000001) {
        if (num > 1 && (sd_scr[0] & 0x02000000)) {
            sd_cmd(0x17020000, num);
            if (sd_err) return 0;
        }
        *EMMC_BLKSIZECNT = (num << 16) | 512;
        sd_cmd(num == 1 ? 0x11220010 : 0x12220032, lba);
        if (sd_err) return 0;
    } else {
        *EMMC_BLKSIZECNT = (1 << 16) | 512;
    }
    while (c < num) {
        if (!(sd_scr[0] & 0x00000001)) {
            sd_cmd(0x11220010, (lba + c) * 512);
            if (sd_err) return 0;
        }
        if ((r = sd_int(0x00000020))) {
            sd_err = r;
            return 0;
        }
        for (d = 0; d < 128; d++) buf[d] = *EMMC_DATA;
        c++;
        buf += 128;
    }
    if (num > 1 && !(sd_scr[0] & 0x02000000) && (sd_scr[0] & 0x00000001)) sd_cmd(0x0C030000, 0);
    return sd_err != 0 || c != num ? 0 : num * 512;
}
