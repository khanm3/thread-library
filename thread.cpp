#include <ucontext.h>
#include "thread.h"
#include "types.h"

thread::thread(thread_startfunc_t body, void *arg) {
	cpu::interrupt_disable();
    // TODO: MULTIPROCESSOR - switch invariant - acquire guard

    // create tcb and put it onto ready queue
    readyQueue.push(Tcb(READY, body, arg));

    // TODO: MULTIPROCESSOR - IPI to available CPU
    // TODO: MULTIPROCESSOR - free guard
    cpu::interrupt_enable();
}

thread::~thread() = default;

void thread::yield() {
    cpu::interrupt_disable();
    // TODO: MULTIPROCESSOR - acquire guard

    if (!readyQueue.empty()) {
        // put current thread on ready queue
        assert(runningList.find(cpu::self()) != runningList.end());
        Tcb &currThread = runningList[cpu::self()];
        currThread.state = READY;
        readyQueue.push(std::move(currThread));

        // put next thread in ready queue onto running list
        readyQueue.front().state = RUNNING;
        currThread = std::move(readyQueue.front());
        readyQueue.pop();

        // switch to next thread
        swapcontext(readyQueue.back().ctx.get(), currThread.ctx.get());
    }

    // TODO: MULTIPROCESSOR - free guard
    cpu::interrupt_enable();
}
