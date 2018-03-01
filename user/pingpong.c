#include "lib.h"

void umain(void) {
    unsigned int who, i;
    if ((who = fork()) != 0) {
        writef("\n@@@@@send 0 from %x to %x\n", syscall_getenvid(), who);
        ipc_send(who, 0, 0, 0);
    }
    for (;;) {
        writef("%x am waiting.....\n", syscall_getenvid());
        i = ipc_recv(&who, 0, 0);
        writef("%x got %d from %x\n", syscall_getenvid(), i, who);
        if (i == 10) {
            return;
        }
        i++;
        writef("\n@@@@@send %d from %x to %x\n", i, syscall_getenvid(), who);
        ipc_send(who, i, 0, 0);
        if (i == 10) {
            return;
        }
    }
}

