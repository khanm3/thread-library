#include <cstdio>
#include <ucontext.h>
#include "cpu.h"
#include "types.h"

void cpu::init(thread_startfunc_t body, void *arg) {
    // TODO: MULTIPROCESSOR: consider when body == nullptr

    // switch invariant - disable interrupts
    assert_interrupts_disabled();
    // TODO: MULTIPROCESSOR - switch invariant - acquire guard

    // create tcb object on running list, and initialize its state and context
    Tcb &threadToRun = runningList[cpu::self()];
    threadToRun.state = RUNNING;
    makecontext(threadToRun.ctx.get(), (void (*)()) os_wrapper, 2, body, arg);

    // begin executing thread
    setcontext(threadToRun.ctx.get());
}
