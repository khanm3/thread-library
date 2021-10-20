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
    interrupt_vector_table[cpu::IPI] = handle_ipi;

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
        /* ALTERNATIVE: use helper function */
        /* note: the debug output with the helper function is less precise */
        // create empty tcbPtr and put it on the running list
        // runningList[cpu::self()];
        // switch to next ready thread if there is one, else suspend
        // switch_to_next_or_suspend(nullptr);


        // if there's a ready thread, switch to it
        if (!readyQueue.empty()) {
            // update cpu state to running
            cpu::self()->impl_ptr->state = CPU_RUNNING;

            // debug
            // std::printf("%p initialized to running\n", (void *) cpu::self());

            // move top tcb from ready queue onto running list
            TcbPtr &threadToRun = runningList[cpu::self()];
            threadToRun = std::move(readyQueue.front());
            readyQueue.pop();
            *(threadToRun->state) = RUNNING;

            // begin executing thread
            setcontext(&threadToRun->ctx);
        }
        // else there's no threads to run, so suspend
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
}
