#include <cstdio>
#include <ucontext.h>
#include "cpu.h"
#include "thread.h"
#include "types.h"

void cpu::init(thread_startfunc_t body, void *arg) {
    // switch invariant - disable interrupts
    assert_interrupts_disabled();
    while (cpu::guard.exchange(true)) {
        /* MT */
    }

    // initialize interrupt vector table
    interrupt_vector_table[cpu::TIMER] = handle_timer;
    interrupt_vector_table[cpu::IPI] = handle_ipi;

    if (body) {
        // initialize kernel suspend thread
        
        // Not convinced this will always work.
        // Do we need to free and re-new the stack between uses?
        suspendCtx->uc_stack.ss_sp = new char[STACK_SIZE];
        suspendCtx->uc_stack.ss_size = STACK_SIZE;
        suspendCtx->uc_stack.ss_flags = 0;
        suspendCtx->uc_link = nullptr;
        makecontext(suspendCtx, (void (*)()) suspend_thread, 0);
        // create tcb object and put it on the running list
        TcbPtr &threadToRun = runningList[cpu::self()];
        threadToRun = TcbPtr(new Tcb(RUNNING, body, arg));

        // begin executing thread
        setcontext(&threadToRun->ctx);
    }
    else {
        // create empty tcb object and put it on running list
        TcbPtr &threadToRun = runningList[cpu::self()];

        // if there's work to do, do it
        if (!readyQueue.empty()) {
            *(readyQueue.front()->state) = RUNNING;
            threadToRun = std::move(readyQueue.front());
            readyQueue.pop();
            setcontext(&threadToRun->ctx);
        }
        // else, suspend
        else {
            cpu::guard.store(false);
            cpu::interrupt_enable_suspend();
        }
    }
}
