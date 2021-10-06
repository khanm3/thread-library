#ifndef _TYPES_H
#define _TYPES_H

#include <map>
#include <memory>
#include <queue>
#include <ucontext.h>
#include "thread.h"

enum ThreadState {
    INITIALIZED,
    RUNNING,
    FINISHED,
    BLOCKED,
    READY,
};

class Tcb {
public:
    // allocate ucontext and stack
    Tcb() :
        ctx(std::unique_ptr<ucontext_t>(new ucontext_t())),
        state(INITIALIZED)
    {
        ctx->uc_stack.ss_sp = new char[STACK_SIZE];
        ctx->uc_stack.ss_size = STACK_SIZE;
        ctx->uc_stack.ss_flags = 0;
        ctx->uc_link = nullptr;
    }

    std::unique_ptr<ucontext_t> ctx;
    ThreadState state;
};

extern std::queue<Tcb> readyQueue;
extern std::vector<Tcb> finishedQueue;
extern std::map<cpu *, Tcb> runningList;

void os_wrapper(thread_startfunc_t, void *);

#endif
