#ifndef OS_SD_H
#define OS_SD_H

int sd_init();
int sd_readblock(unsigned int lba, unsigned char *buffer, unsigned int num);

#endif //OS_SD_H
