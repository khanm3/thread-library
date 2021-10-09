#include <cassert>
#include "types.h"

std::queue<TcbPtr> readyQueue;
std::vector<TcbPtr> finishedList;
std::map<cpu *, TcbPtr> runningList;

Tcb::Tcb()
    : ctx(ucontext_t())
    , state(ThreadStatePtr(new ThreadState(INITIALIZED)))
    , joinQueue(JoinQueuePtr(nullptr))
{

}

Tcb::Tcb(ThreadState state, thread_startfunc_t body, void *arg)
    : ctx(ucontext_t())
    , state(ThreadStatePtr(new ThreadState(state)))
    , joinQueue(JoinQueuePtr(new std::queue<TcbPtr>()))
{
    ctx.uc_stack.ss_sp = new char[STACK_SIZE];
    ctx.uc_stack.ss_size = STACK_SIZE;
    ctx.uc_stack.ss_flags = 0;
    ctx.uc_link = nullptr;
    makecontext(&ctx, (void (*)()) os_wrapper, 2, body, arg);
}

void Tcb::freeStack() {
    assert(ctx.uc_stack.ss_sp != nullptr);
    delete[] (char *) ctx.uc_stack.ss_sp;
    ctx.uc_stack.ss_sp = nullptr;
}

void os_wrapper(thread_startfunc_t body, void *arg) {
    assert_interrupts_disabled();
    // do os stuff

    // If there are any finished threads to clean up, clean them up
    while(!finishedList.empty()) {
        finishedList.back()->freeStack();
        finishedList.pop_back();
    }

    // enable interrupts - switch invariant
    cpu::interrupt_enable();

    // TODO: MULTIPROCESSOR - switch invariant - acquire guard

    // run thread to finish
    body(arg);

    // disable interrupts - switch invariant
    cpu::interrupt_disable();

    // get tcb of currently running thread
    assert(runningList.find(cpu::self()) != runningList.end());
    TcbPtr &currThread = runningList[cpu::self()];

    // move all tcbs on join queue onto ready queue
    while (!currThread->joinQueue->empty()) {
        *(currThread->joinQueue->front()->state) = READY;
        readyQueue.push(std::move(currThread->joinQueue->front()));
        currThread->joinQueue->pop();
    }

    // move tcb of currently running thread to finished list
    *(currThread->state) = FINISHED;
    finishedList.push_back(std::move(currThread));

    // if another thread on ready queue, switch to it
    if (!readyQueue.empty()) {
        // move top tcb from ready queue onto running list
        currThread = std::move(readyQueue.front());
        readyQueue.pop();
        *(currThread->state) = RUNNING;

        // switch context to new current thread
        // concern: currThread might go out of context and then we would leak its stack
        setcontext(&currThread->ctx);
    }
    // else no threads to run, put cpu to sleep
    else {
        cpu::interrupt_enable_suspend();
    }
}
