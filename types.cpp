#include <cassert>
#include "types.h"

std::queue<TcbPtr> readyQueue;
std::vector<TcbPtr> finishedList;
std::map<cpu *, TcbPtr> runningList;
ucontext_t *dummyCtx = new ucontext_t;
ucontext_t *suspendCtx = new ucontext_t;

Tcb::Tcb()
    : ctx(ucontext_t())
    , stackPtr(nullptr)
    , state(ThreadStatePtr(new ThreadState(INITIALIZED)))
    , joinQueue(JoinQueuePtr(nullptr))
{

}

Tcb::Tcb(ThreadState state, thread_startfunc_t body, void *arg)
    : ctx(ucontext_t())
    , stackPtr(new char[STACK_SIZE])
    , state(ThreadStatePtr(new ThreadState(state)))
    , joinQueue(JoinQueuePtr(new std::queue<TcbPtr>()))
{
    ctx.uc_stack.ss_sp = stackPtr;
    ctx.uc_stack.ss_size = STACK_SIZE;
    ctx.uc_stack.ss_flags = 0;
    ctx.uc_link = nullptr;
    makecontext(&ctx, (void (*)()) os_wrapper, 2, body, arg);
}

void Tcb::freeStack() {
    assert(stackPtr != nullptr);
    delete[] (char *) stackPtr;
    stackPtr = nullptr;
}

RaiiLock::RaiiLock() {
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    while (cpu::guard.exchange(true)) {
        /* MT */
    }
}

RaiiLock::~RaiiLock() {
    assert_interrupts_disabled();
    cpu::guard.store(false);
    cpu::interrupt_enable();
}

void os_wrapper(thread_startfunc_t body, void *arg) {
    assert_interrupts_disabled();
    // do os stuff

    // if there are any threads on the finished list, clean them up
    cleanup_finished_list();

    // enable interrupts - switch invariant
    cpu::guard.store(false);
    cpu::interrupt_enable();

    // run thread to finish
    body(arg);

    // disable interrupts - switch invariant
    cpu::interrupt_disable();
    while (cpu::guard.exchange(true)) {
        /* MT */
    }

    // get tcb of currently running thread
    assert(runningList.find(cpu::self()) != runningList.end());
    TcbPtr &currThread = runningList[cpu::self()];

    // move all tcbs on join queue onto ready queue
    while (!currThread->joinQueue->empty()) {
        *(currThread->joinQueue->front()->state) = READY;
        readyQueue.push(std::move(currThread->joinQueue->front()));
        currThread->joinQueue->pop();
    }

    // TODO: send IPI
    send_ipi();

    // move tcb of currently running thread to finished list
    *(currThread->state) = FINISHED;
    finishedList.push_back(std::move(currThread));

    // switch to next ready thread if there is one, else suspend
    switch_to_next_or_suspend(dummyCtx);
}

void switch_to_next_or_suspend(ucontext_t *saveloc) {
    assert_interrupts_disabled();

    assert(runningList.find(cpu::self()) != runningList.end());
    TcbPtr &currThread = runningList[cpu::self()];
    // TODO: assert currThread points to an "empty" Tcb

    // if another thread on ready queue, switch to it
    if (!readyQueue.empty()) {
        // move top tcb from ready queue onto running list
        currThread = std::move(readyQueue.front());
        readyQueue.pop();
        *(currThread->state) = RUNNING;

        // switch context to new current thread
        swapcontext(saveloc, &currThread->ctx);
    }
    // else no other threads, save tcb to saveloc then suspend
    else {
        // switch to kernel suspend thread
        swapcontext(saveloc, suspendCtx);
    }

    assert_interrupts_disabled();
}

void cleanup_finished_list() {
    assert_interrupts_disabled();

    while(!finishedList.empty()) {
        finishedList.back()->freeStack();
        finishedList.pop_back();
    }
}

void yield_helper() {
    // there is a next ready thread
    if (!readyQueue.empty()) {
        // put current thread on ready queue
        assert(runningList.find(cpu::self()) != runningList.end());
        TcbPtr &currThread = runningList[cpu::self()];
        *(currThread->state) = READY;
        readyQueue.push(std::move(currThread));

        // TODO: send IPI: do we need to send an IPI here?

        // switch to next ready thread
        switch_to_next_or_suspend(&readyQueue.back()->ctx);

        // if there are any threads on the finished list, clean them up
        cleanup_finished_list();
    }
}

void handle_timer() {
    RaiiLock l;
    // TODO: MULTIPROCESSOR - acquire guard

    yield_helper();

    // TODO: MULTIPROCESSOR - free guard
}

void send_ipi() {
    assert_interrupts_disabled();
    assert(cpu::guard);

    // send IPIs if there's work to do
    if (!readyQueue.empty()) {
        // loop through list of CPUs
        size_t i = 0;
        for (auto& cpuTcbPair : runningList) {
            if (i >= readyQueue.size()) {
                break;
            }
            // send IPI if CPU is suspended
            if (!cpuTcbPair.second) {
                cpuTcbPair.first->interrupt_send();
                ++i;
            }
        }
    }
}

void handle_ipi() {
    RaiiLock l;

    // if there's work to do, do it
    if (!readyQueue.empty()) {
        TcbPtr &threadToRun = runningList[cpu::self()];
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

void suspend_thread() {
    cpu::guard.store(false);
    cpu::interrupt_enable_suspend();
}
