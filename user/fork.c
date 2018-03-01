#include "lib.h"

extern struct Env *env;

int fork() {
    unsigned int envid = syscall_fork();
    if (envid == 0) {
        env = &envs[ENVX(syscall_getenvid())];
    }
    return envid;
}
