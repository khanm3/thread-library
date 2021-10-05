#include <ucontext.h>
#include "types.h"

std::queue<Tcb> readyQueue;

void os_wrapper(thread_startfunc_t body, void *arg) {
    // do os stuff

    // enable interrupts - switch invariant
    cpu::interrupt_enable();
    // If there are any finished threads to clean up, clean them up
    // TODO: MULTIPROCESSOR - switch invariant - acquire guard

    // run thread to finish
    body(arg);

    // disable interrupts - switch invariant
    cpu::interrupt_disable();

    // if another thread on ready queue, switch to it
    if (!readyQueue.empty()) {
        Tcb threadToRun;
        threadToRun = std::move(readyQueue.front());
        readyQueue.pop();

        setcontext(threadToRun.ctx.get());
    }
}
