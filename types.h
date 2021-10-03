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
    std::unique_ptr<ucontext_t> ctx;
    ThreadState state;
};

extern std::queue<Tcb> readyQueue;

void os_wrapper(thread_startfunc_t, void *);

#endif
