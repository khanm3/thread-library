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
	RaiiLock l;
    // TODO: MULTIPROCESSOR - switch invariant - acquire guard
    impl_ptr = new impl();

    // create tcb and put it onto ready queue
    readyQueue.push(TcbPtr(new Tcb(READY, body, arg)));
    impl_ptr->state = readyQueue.back()->state;
    impl_ptr->joinQueue = readyQueue.back()->joinQueue;
    // TODO: send IPI
    send_ipi();
    // TODO: MULTIPROCESSOR - free guard
}

thread::~thread() {
    delete impl_ptr;
}

void thread::yield() {
    RaiiLock l;
    // TODO: MULTIPROCESSOR - acquire guard

    yield_helper();

    // TODO: MULTIPROCESSOR - free guard
}

void thread::join() {
    RaiiLock l;
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

        // switch to next ready thread if there is one, else suspend
        switch_to_next_or_suspend(&joinQueue->back()->ctx);

        // if there are any threads on the finished list, clean them up
        cleanup_finished_list();
    }
    // case 2 thread has finished, continue execution
    else {
        // do nothing
    }

    // TODO: MULTIPROCESSOR - free guard
}
