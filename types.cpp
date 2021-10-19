#include <cassert>
#include "types.h"

std::queue<TcbPtr> readyQueue;
std::vector<TcbPtr> finishedList;
std::map<cpu *, TcbPtr> runningList;
ucontext_t *dummyCtx = new ucontext_t;

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
    // TODO: MULTIPROCESSOR: acquire guard
}

RaiiLock::~RaiiLock() {
    assert_interrupts_disabled();

    // TODO: MULTIPROCESSOR: release guard
    cpu::interrupt_enable();
}

void os_wrapper(thread_startfunc_t body, void *arg) {
    assert_interrupts_disabled();

    // if there are any threads on the finished list, clean them up
    cleanup_finished_list();

    // switch invariant - release lock on processor before running user code
    // TODO: MULTIPROCESSOR - release guard
    cpu::interrupt_enable();

    // run user code
    body(arg);

    // switch invariant - acquire lock on processor before handling shared
    //                    data structures + switching
    cpu::interrupt_disable();
    // TODO: MULTIPROCESSOR - acquire guard

    // get tcb of currently running thread
    assert(runningList.find(cpu::self()) != runningList.end());
    TcbPtr &currThread = runningList[cpu::self()];

    // move all tcbs on join queue onto ready queue
    while (!currThread->joinQueue->empty()) {
        *(currThread->joinQueue->front()->state) = READY;
        readyQueue.push(std::move(currThread->joinQueue->front()));
        currThread->joinQueue->pop();
    }

    // TODO: MULTIPROCESSOR - send IPI

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
        // TODO: multiprocessor - update tcb
        // getcontext(saveloc);
        // increment PC counter to be directly after suspend
        cpu::interrupt_enable_suspend();
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
    assert_interrupts_disabled();

    // there is a next ready thread
    if (!readyQueue.empty()) {
        // put current thread on ready queue
        assert(runningList.find(cpu::self()) != runningList.end());
        TcbPtr &currThread = runningList[cpu::self()];
        *(currThread->state) = READY;
        readyQueue.push(std::move(currThread));

        // switch to next ready thread
        switch_to_next_or_suspend(&readyQueue.back()->ctx);

        // switch invariant - assert interrupts disabled after returning from switch
        assert_interrupts_disabled();

        // if there are any threads on the finished list, clean them up
        cleanup_finished_list();
    }
}

void handle_timer() {
    RaiiLock l;
    yield_helper();
}
