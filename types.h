#ifndef _TYPES_H
#define _TYPES_H

#include <cassert>
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
    // EFFECTS: creates a TCB with a ucontext_t and stack already allocated
    // and its state set to INITIALIZED
    Tcb() :
        ctx(std::unique_ptr<ucontext_t>(new ucontext_t())),
        state(INITIALIZED)
    {
        ctx->uc_stack.ss_sp = new char[STACK_SIZE];
        ctx->uc_stack.ss_size = STACK_SIZE;
        ctx->uc_stack.ss_flags = 0;
        ctx->uc_link = nullptr;
    }

    // REQUIRES: ctx manages a ucontext_t object
    // EFFECTS: deallocates stack
    void freeStack() {
        assert(ctx);
        delete[] (char *) ctx->uc_stack.ss_sp;
    }

    // use default move constructors
    Tcb(Tcb&&) = default;
    Tcb& operator=(Tcb&&) = default;

    // disable copy constructors
    Tcb(const Tcb&) = delete;
    Tcb& operator=(const Tcb&) = delete;

    std::unique_ptr<ucontext_t> ctx;
    ThreadState state;
};

extern std::queue<Tcb> readyQueue;
extern std::vector<Tcb> finishedQueue;
extern std::map<cpu *, Tcb> runningList;

void os_wrapper(thread_startfunc_t, void *);

#endif
