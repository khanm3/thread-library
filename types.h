#ifndef _TYPES_H
#define _TYPES_H

#include <memory>
#include <queue>
#include <ucontext.h>
#include "thread.h"

enum ThreadState {
    RUNNING,
    FINISHED,
    BLOCKED,
    READY,
};

class Tcb {
public:
    
    Tcb() {
        stack = std::unique_ptr<char>(new char[STACK_SIZE]);
        ctx = std::unique_ptr<ucontext_t>(new ucontext_t());
        ctx->uc_stack.ss_sp = stack.get();
        ctx->uc_stack.ss_size = STACK_SIZE;
        ctx->uc_stack.ss_flags = 0;
        ctx->uc_link = nullptr;
        state = ThreadState::READY;
    }
    
    std::unique_ptr<ucontext_t> ctx;
    ThreadState state;
    std::unique_ptr<char> stack;
};

extern std::queue<Tcb> readyQueue;
extern std::vector<Tcb> finishedQueue;

void os_wrapper(thread_startfunc_t, void *);

#endif
