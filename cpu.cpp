#include <cstdio>
#include <ucontext.h>
#include "cpu.h"
#include "types.h"

void cpu::init(thread_startfunc_t body, void *arg) {
    // TODO: MULTIPROCESSOR: consider when body == nullptr

    // switch invariant - disable interrupts
    // interrupt_disable(); When we disable interrupts here we are double disabling

    // TODO: MULTIPROCESSOR - switch invariant - acquire guard

    // create temp tcb object and initialize it
    Tcb threadToRun;

    // switch invariant
    assert_interrupts_disabled();

    makecontext(threadToRun.ctx.get(), (void (*)()) os_wrapper, 2, body, arg);
    setcontext(threadToRun.ctx.get());
}
