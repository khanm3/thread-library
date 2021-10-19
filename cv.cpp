#include <cassert>
#include "cv.h"
#include "mutexImpl.h"
#include "types.h"

class cv::impl {
public:
    std::queue<TcbPtr> waitQueue;
};

cv::cv() {
    impl_ptr = new impl();
}

cv::~cv() {
    delete impl_ptr;
}

void cv::wait(mutex &m) {
	RaiiLock l;

    // release m
    // if the current thread doesn't hold m, unlockHelper will throw an error
    m.impl_ptr->unlockHelper();

    assert(runningList.find(cpu::self()) != runningList.end());
    TcbPtr &currThread = runningList[cpu::self()];

    // move the current thread to the wait queue
    *(currThread->state) = BLOCKED;
    impl_ptr->waitQueue.push(std::move(currThread));

    // switch to next ready thread if there is one, else suspend
    switch_to_next_or_suspend(&impl_ptr->waitQueue.back()->ctx);

    // switch invariant - assert interrupts disabled after returning from switch
    assert_interrupts_disabled();

    // if there are any threads on the finished list, clean them up
    cleanup_finished_list();

    // upon wakeup, attempt to reacquire m
    m.impl_ptr->lockHelper();
}

void cv::signal() {
	RaiiLock l;

    // move the next waiting thread to the ready queue if there is one
    if (!impl_ptr->waitQueue.empty()) {
        *(impl_ptr->waitQueue.front()->state) = READY;
        readyQueue.push(std::move(impl_ptr->waitQueue.front()));
        impl_ptr->waitQueue.pop();

        // TODO: MULTIPROCESSOR - send IPI
    }
}

void cv::broadcast() {
	RaiiLock l;

    // move all waiting threads to the ready queue if there are any
    while (!impl_ptr->waitQueue.empty()) {
        *(impl_ptr->waitQueue.front()->state) = READY;
        readyQueue.push(std::move(impl_ptr->waitQueue.front()));
        impl_ptr->waitQueue.pop();
    }

    // TODO: MULTIPROCESSOR - send IPI
}
