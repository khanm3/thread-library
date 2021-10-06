#include "types.h"

std::queue<Tcb> readyQueue;
std::vector<Tcb> finishedList;
std::map<cpu *, Tcb> runningList;

void os_wrapper(thread_startfunc_t body, void *arg) {
    assert_interrupts_disabled();
    // do os stuff

    // If there are any finished threads to clean up, clean them up
    while(!finishedList.empty()) {
        delete[] (char*) finishedList.back().ctx->uc_stack.ss_sp;
        finishedList.pop_back();
    }

    // enable interrupts - switch invariant
    cpu::interrupt_enable();

    // TODO: MULTIPROCESSOR - switch invariant - acquire guard

    // run thread to finish
    body(arg);

    // disable interrupts - switch invariant
    cpu::interrupt_disable();

    // move tcb of currently running thread to finished list
    Tcb &currThread = runningList[cpu::self()];
    finishedList.push_back(std::move(currThread));

    // if another thread on ready queue, switch to it
    if (!readyQueue.empty()) {
        // move top tcb on ready queue onto running list
        currThread = std::move(readyQueue.front());
        readyQueue.pop();

        // switch context to new current thread
        // concern: currThread might go out of context and then we would leak its stack
        setcontext(currThread.ctx.get());
    }
}
