#ifndef _TYPES_H
#define _TYPES_H

#include <memory>
#include <queue>
#include <ucontext.h>
#include "thread.h"

enum ThreadState {
    RUNNING,
};

class Tcb {
public:
    /*
    Tcb() {
        char *stack = new char[STACK_SIZE];
        ctx = std::unique_ptr<ucontext_t>(new ucontext_t());
        ctx->uc_stack.ss_sp = stack;
        ctx->uc_stack.ss_size = STACK_SIZE;
        ctx->uc_stack.ss_flags = 0;
        ctx->uc_link = nullptr;
    }
    */
    std::unique_ptr<ucontext_t> ctx;
    ThreadState state;
};

extern std::queue<Tcb> readyQueue;

void os_wrapper(thread_startfunc_t, void *);

#endif
