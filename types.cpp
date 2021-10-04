#include <ucontext.h>
#include "types.h"

std::queue<Tcb> ready_queue;

void os_wrapper(thread_startfunc_t body, void *arg) {
    // do os stuff

    // enable interrupts - switch invariant
    cpu::interrupt_enable();

    // TODO: MULTIPROCESSOR - switch invariant - acquire guard

    // run thread to finish
    body(arg);

    // disable interrupts - switch invariant
    cpu::interrupt_disable();

    // if another thread on ready queue, switch to it
    if (!ready_queue.empty()) {
        Tcb threadToRun;
        threadToRun = std::move(ready_queue.front());
        ready_queue.pop();

        setcontext(threadToRun.ctx.get());
    }
}
