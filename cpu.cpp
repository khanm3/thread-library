#include <cstdio>
#include <ucontext.h>
#include "cpu.h"
#include "cpuImpl.h"
#include "types.h"

void cpu::init(thread_startfunc_t body, void *arg) {
    // switch invariant - acquire lock on processor before handling shared
    //                    data structures + switching
    assert_interrupts_disabled(); // interrupts start out disabled, so assert instead
    RaiiLock::acquireGuard();

    // initialize interrupt vector table
    interrupt_vector_table[cpu::TIMER] = handle_timer;
    // TODO: implement IPI interrupt handler

    // allocate impl_ptr
    impl_ptr = new impl();

    if (body) {
        // set cpu state to running
        impl_ptr->state = CPU_RUNNING;

        // debug
        // printf("%p initialized to running\n", cpu::self());

        // create tcb object and put it on the running list
        TcbPtr &threadToRun = runningList[cpu::self()];
        threadToRun = TcbPtr(new Tcb(RUNNING, body, arg));

        // begin executing thread
        setcontext(&threadToRun->ctx);
    }
    else {
        // set cpu state to suspended
        impl_ptr->state = CPU_SUSPENDED;

        // debug
        // printf("%p initialized to suspended\n", cpu::self());

        // create empty tcbptr (unique pointer that doesn't manage a resource)
        // and put it on the running list
        runningList[cpu::self()] = TcbPtr();

        // begin executing suspend thread
        setcontext(impl_ptr->suspendCtx);
    }
}
