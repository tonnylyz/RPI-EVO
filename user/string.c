#include "lib.h"

int strlen(const char *s) {
    int n;
    for (n = 0; *s; s++) {
        n++;
    }
    return n;
}

char *strcpy(char *dst, const char *src) {
    char *ret;
    ret = dst;
    while ((*dst++ = *src++) != 0);
    return ret;
}

const char *strchr(const char *s, char c) {
    for (; *s; s++)
        if (*s == c) {
            return s;
        }
    return 0;
}

void *memcpy(void *destaddr, void const *srcaddr, unsigned long len) {
    char *dest = destaddr;
    char const *src = srcaddr;
    while (len-- > 0) {
        *dest++ = *src++;
    }
    return destaddr;
}


int strcmp(const char *p, const char *q) {
    int i;
    for (i = 0;; i++) {
        if (p[i] != q[i]) {
            return p[i] < q[i] ? -1 : 1;
        }
        if (p[i] == '\0') {
            return 0;
        }
    }
}


void user_bcopy(const void *src, void *dst, unsigned long len) {
    unsigned long max = (unsigned long)dst + len;
    while ((unsigned long) dst + 7 < max) {
        *(unsigned long *) dst = *(unsigned long *) src;
        dst += 8;
        src += 8;
    }
    while ((unsigned long) dst < max) {
        *(unsigned char *) dst = *(unsigned char *) src;
        dst += 1;
        src += 1;
    }
}

void user_bzero(void *b, unsigned long len) {
    unsigned long max = (unsigned long)b + len;
    while ((unsigned long) b + 7 < max) {
        *(unsigned long *) b = 0;
        b += 8;
    }
    while ((unsigned long) b < max) {
        *(unsigned char *) b++ = 0;
    }
}
