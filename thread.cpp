#include <cassert>
#include <ucontext.h>
#include "thread.h"
#include "types.h"

using ThreadStateWeakPtr = std::weak_ptr<ThreadState>;
using JoinQueueWeakPtr = std::weak_ptr<std::queue<TcbPtr>>;

class thread::impl {
    public:
        ThreadStateWeakPtr state;
        JoinQueueWeakPtr joinQueue;
};

thread::thread(thread_startfunc_t body, void *arg) {
	cpu::interrupt_disable();
    // TODO: MULTIPROCESSOR - switch invariant - acquire guard
    impl_ptr = new impl();

    // create tcb and put it onto ready queue
    readyQueue.push(TcbPtr(new Tcb(READY, body, arg)));
    impl_ptr->state = readyQueue.back()->state;
    impl_ptr->joinQueue = readyQueue.back()->joinQueue;
    // TODO: MULTIPROCESSOR - IPI to available CPU
    // TODO: MULTIPROCESSOR - free guard
    cpu::interrupt_enable();
}

thread::~thread() {
    delete impl_ptr;
}

void thread::yield() {
    cpu::interrupt_disable();
    // TODO: MULTIPROCESSOR - acquire guard

    if (!readyQueue.empty()) {
        // put current thread on ready queue
        assert(runningList.find(cpu::self()) != runningList.end());
        TcbPtr &currThread = runningList[cpu::self()];
        *(currThread->state) = READY;
        readyQueue.push(std::move(currThread));

        // put next thread in ready queue onto running list
        *(readyQueue.front()->state) = RUNNING;
        currThread = std::move(readyQueue.front());
        readyQueue.pop();

        // switch to next thread
        swapcontext(&readyQueue.back()->ctx, &currThread->ctx);
    }

    // TODO: MULTIPROCESSOR - free guard
    cpu::interrupt_enable();
}

void thread::join() {
    cpu::interrupt_disable();
    // TODO: MULTIPROCESSOR - acquire guard

    // TODO: check currThread TCB is valid (state, joinQueue, stack)
    assert(runningList.find(cpu::self()) != runningList.end());
    TcbPtr &currThread = runningList[cpu::self()];

    ThreadStatePtr state = impl_ptr->state.lock();
    // case 1: thread is still running, block current thread
    if (state && *state != FINISHED) {
        JoinQueuePtr joinQueue = impl_ptr->joinQueue.lock();
        assert(joinQueue);

        // move current thread to join queue
        *(currThread->state) = BLOCKED;
        joinQueue->push(std::move(currThread));

        // TODO: abstract this part out?
        // TODO: make order of state update consistent (before moving TCB)
        // if another thread on ready queue, switch to it
        if (!readyQueue.empty()) {
            // move top tcb from ready queue onto running list
            currThread = std::move(readyQueue.front());
            readyQueue.pop();
            *(currThread->state) = RUNNING;

            // switch context to new current thread
            swapcontext(&joinQueue->back()->ctx, &currThread->ctx);
        }
        // else no other threads, update TCB on join Queue then suspend
        else {
            // TODO: multiprocessor - update TCB
            // getcontext(&joinQueue->back()->ctx);
            // increment PC counter to be directly after suspend
            cpu::interrupt_enable_suspend();
        }

        // TODO: insert a "switching into new thread" invariant function
        // check state (thread finished) invariant
        assert(!state || *state == FINISHED);


    }
    // case 2 thread has finished, continue execution
    else {
        // do nothing
    }

    // TODO: MULTIPROCESSOR - free guard
    cpu::interrupt_enable();
}
