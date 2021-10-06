#include <cassert>
#include "types.h"

std::queue<Tcb> readyQueue;
std::vector<Tcb> finishedList;
std::map<cpu *, Tcb> runningList;

Tcb::Tcb()
    : ctx(std::unique_ptr<ucontext_t>(nullptr))
    , state(INITIALIZED)
{
}

Tcb::Tcb(ThreadState state, thread_startfunc_t body, void *arg)
    : ctx(std::unique_ptr<ucontext_t>(new ucontext_t()))
    , state(state)
{
    ctx->uc_stack.ss_sp = new char[STACK_SIZE];
    ctx->uc_stack.ss_size = STACK_SIZE;
    ctx->uc_stack.ss_flags = 0;
    ctx->uc_link = nullptr;
    makecontext(ctx.get(), (void (*)()) os_wrapper, 2, body, arg);
}

void Tcb::freeStack() {
        assert(ctx);
        delete[] (char *) ctx->uc_stack.ss_sp;
}

void os_wrapper(thread_startfunc_t body, void *arg) {
    assert_interrupts_disabled();
    // do os stuff

    // If there are any finished threads to clean up, clean them up
    while(!finishedList.empty()) {
        finishedList.back().freeStack();
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
    currThread.state = FINISHED;
    finishedList.push_back(std::move(currThread));

    // if another thread on ready queue, switch to it
    if (!readyQueue.empty()) {
        // move top tcb from ready queue onto running list
        currThread = std::move(readyQueue.front());
        readyQueue.pop();
        currThread.state = RUNNING;

        // switch context to new current thread
        // concern: currThread might go out of context and then we would leak its stack
        setcontext(currThread.ctx.get());
    }
}
