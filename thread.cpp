#include <ucontext.h>
#include "cpu.h"
#include "types.h"

// extern std::queue<Tcb> readyQueue;

thread::thread(thread_startfunc_t body, void *args) {
	// TODO: MULTIPROCESSOR - switch invariant - acquire guard
	cpu::interrupt_disable();

	// Initialize TCB
	// I feel like this could be modularized
	Tcb readyThread;
	char *stack = new char[STACK_SIZE];
    readyThread.ctx = std::unique_ptr<ucontext_t>(new ucontext_t());
    readyThread.ctx->uc_stack.ss_sp = stack;
    readyThread.ctx->uc_stack.ss_size = STACK_SIZE;
    readyThread.ctx->uc_stack.ss_flags = 0;
    readyThread.ctx->uc_link = nullptr;

    // Add stack pointer and args to contact
    makecontext(readyThread.ctx.get(), (void (*)()) os_wrapper, 2, body, args);

    // Put TCB on readyqueue
    // readyThread.state = READY;
    readyQueue.push(std::move(readyThread));

    // TODO: MULTIPROCESSOR - IPI to available CPU
    // TODO: MULTIPROCESSOR - free guard
    cpu::interrupt_enable();
}

thread::~thread() {
	printf("Thread has died\n");
}
