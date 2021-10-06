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
