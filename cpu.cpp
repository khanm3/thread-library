#include <cstdio>
#include <ucontext.h>
#include "cpu.h"
#include "types.h"

void cpu::init(thread_startfunc_t body, void *arg) {
    // TODO: MULTIPROCESSOR: consider when body == nullptr

    // switch invariant - acquire lock on processor before handling shared
    //                    data structures + switching
    assert_interrupts_disabled(); // interrupts start out disabled, so assert instead
    // TODO: MULTIPROCESSOR - acquire guard

    // initialize interrupt vector table
    interrupt_vector_table[cpu::TIMER] = handle_timer;
    // TODO: implement IPI interrupt handler

    // create tcb object and put it on the running list
    TcbPtr &threadToRun = runningList[cpu::self()];
    threadToRun = TcbPtr(new Tcb(RUNNING, body, arg));

    // begin executing thread
    setcontext(&threadToRun->ctx);
}
