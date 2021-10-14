#include <cstdio>
#include <ucontext.h>
#include "cpu.h"
#include "types.h"

void cpu::init(thread_startfunc_t body, void *arg) {
    // TODO: MULTIPROCESSOR: consider when body == nullptr

    // switch invariant - disable interrupts
    assert_interrupts_disabled();
    // TODO: MULTIPROCESSOR - switch invariant - acquire guard

    // initialize interrupt vector table
    // TODO: implement IPI interrupt handler
    interrupt_vector_table[cpu::TIMER] = handle_timer;

    // create tcb object and put it on the running list
    TcbPtr &threadToRun = runningList[cpu::self()];
    threadToRun = TcbPtr(new Tcb(RUNNING, body, arg));

    // begin executing thread
    setcontext(&threadToRun->ctx);
}
