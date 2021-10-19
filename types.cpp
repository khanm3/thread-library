#include <cassert>
#include <cstdio>
#include "cpuImpl.h"
#include "types.h"

std::queue<TcbPtr> readyQueue;
std::vector<TcbPtr> finishedList;
std::map<cpu *, TcbPtr> runningList;

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
    lock();
}

RaiiLock::~RaiiLock() {
    assert_interrupts_disabled();
    unlock();
}

void RaiiLock::lock() {
    cpu::interrupt_disable();
    acquireGuard();
}

void RaiiLock::unlock() {
    releaseGuard();
    cpu::interrupt_enable();
}

void RaiiLock::acquireGuard() {
    while (cpu::guard.exchange(true)) {
        // spinlock
    }
}

void RaiiLock::releaseGuard() {
    cpu::guard.store(false);
}

void os_wrapper(thread_startfunc_t body, void *arg) {
    assert_interrupts_disabled();

    // if there are any threads on the finished list, clean them up
    cleanup_finished_list();

    // switch invariant - release lock on processor before running user code
    RaiiLock::unlock();

    // run user code
    body(arg);

    // switch invariant - acquire lock on processor before handling shared
    //                    data structures + switching
    RaiiLock::lock();

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
    switch_to_next_or_suspend(nullptr);
}

void switch_to_next_or_suspend(ucontext_t *saveloc) {
    assert_interrupts_disabled();

    assert(runningList.find(cpu::self()) != runningList.end());
    TcbPtr &currThread = runningList[cpu::self()];
    // TODO: assert currThread points to an "empty" Tcb

    // if another thread on ready queue, switch to it
    if (!readyQueue.empty()) {
        // update cpu state to running
        cpu::self()->impl_ptr->state = CPU_RUNNING;

        // move top tcb from ready queue onto running list
        currThread = std::move(readyQueue.front());
        readyQueue.pop();
        *(currThread->state) = RUNNING;

        // if saveloc is valid, update saveloc with current registers
        // and swap context to new current thread
        if (saveloc) {
            swapcontext(saveloc, &currThread->ctx);

            // switch invariant - assert interrupts disabled after returning from switch
            assert_interrupts_disabled();
        }
        // else, discard current registers and set context to new current thread
        else {
            setcontext(&currThread->ctx);
        }
    }
    // else no other threads, save tcb to saveloc then suspend
    else {
        // update cpu state to suspended
        cpu::self()->impl_ptr->state = CPU_SUSPENDED;

        // put empty tcb ptr (unique pointer that doesn't manage a resource)
        // onto running list
        currThread = TcbPtr();

        // if saveloc is valid, update saveloc with current registers
        // and swap context to suspend thread
        if (saveloc) {
            swapcontext(saveloc, cpu::self()->impl_ptr->suspendCtx);

            // switch invariant - assert interrupts disabled after returning from switch
            assert_interrupts_disabled();
        }
        // else, discard current registers and set context to suspend thread
        else {
            setcontext(cpu::self()->impl_ptr->suspendCtx);
        }
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

void os_suspend() {
    while (true) {
        assert_interrupts_disabled();

        // debug
        // std::printf("%p suspend\n", (void *) cpu::self());

        // switch invariant - release lock on processor before suspending
        RaiiLock::releaseGuard();
        cpu::interrupt_enable_suspend();
    }
}
