#include "types.h"

std::queue<Tcb> readyQueue;
std::vector<Tcb> finishedList;

void os_wrapper(thread_startfunc_t body, void *arg) {
    assert_interrupts_disabled();
    // do os stuff

    // If there are any finished threads to clean up, clean them up
    while(!finishedList.empty()) {
        finishedList.pop_back();
    }

    // enable interrupts - switch invariant
    cpu::interrupt_enable();
    // TODO: MULTIPROCESSOR - switch invariant - acquire guard

    // run thread to finish
    body(arg);

    // disable interrupts - switch invariant
    cpu::interrupt_disable();

    // TODO: Move current tcb from runningList to finishedList

    // if another thread on ready queue, switch to it
    if (!readyQueue.empty()) {
        Tcb threadToRun;
        threadToRun = std::move(readyQueue.front());
        readyQueue.pop();

        // concern: threadToRun might go out of context and then we would leak its stack
        setcontext(threadToRun.ctx.get());
    }
}
