#include <ucontext.h>
#include "thread.h"
#include "types.h"

thread::thread(thread_startfunc_t body, void *args) {
	cpu::interrupt_disable();
    // TODO: MULTIPROCESSOR - switch invariant - acquire guard

	// create temp tcb and initialize its state and context
	Tcb readyThread;
    readyThread.state = READY;
    makecontext(readyThread.ctx.get(), (void (*)()) os_wrapper, 2, body, args);

    // move tcb onto ready queue
    readyQueue.push(std::move(readyThread));

    // TODO: MULTIPROCESSOR - IPI to available CPU
    // TODO: MULTIPROCESSOR - free guard
    cpu::interrupt_enable();
}

thread::~thread() = default;
