#include "lib.h"

extern struct Env *env;

void ipc_send(unsigned int whom, unsigned long val, unsigned long srcva, unsigned long perm) {
    int r;
    while ((r = syscall_ipc_can_send(whom, val, srcva, perm)) == -E_IPC_NOT_RECV) {
        syscall_yield();
    }
    if (r == 0) {
        return;
    }
    user_panic("error in ipc_send: %d", r);
}

unsigned long ipc_recv(unsigned int *whom, unsigned long dstva, unsigned long *perm) {
    syscall_ipc_recv(dstva);
    if (whom) {
        *whom = env->env_ipc_from;
    }
    if (perm) {
        *perm = env->env_ipc_perm;
    }
    return env->env_ipc_value;
}

