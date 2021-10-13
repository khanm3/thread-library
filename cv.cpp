#include <cassert>
#include "cv.h"
#include "types.h"
#include "mutexImpl.h"

class cv::impl {
    public:
    std::queue<TcbPtr> waitQueue;
};

cv::cv() {
    impl_ptr = new impl;
    impl_ptr->waitQueue = std::queue<TcbPtr>();
}

cv::~cv() {
    delete impl_ptr;
}

void cv::wait(mutex &m) {
    // Mutex checks for proper owner
    cpu::interrupt_disable();

    m.impl_ptr->unlockHelper();
    
    assert(runningList.find(cpu::self()) != runningList.end());
    TcbPtr &currThread = runningList[cpu::self()];
    impl_ptr->waitQueue.push(std::move(currThread));

    switch_to_next_or_suspend(&impl_ptr->waitQueue.back()->ctx);

    m.impl_ptr->lockHelper();

    cpu::interrupt_enable();
}

void cv::signal() {
    cpu::interrupt_disable();

    if (!impl_ptr->waitQueue.empty()) {
        TcbPtr &threadToRun = impl_ptr->waitQueue.front();
        readyQueue.push(std::move(threadToRun));
        impl_ptr->waitQueue.pop();
    }

    cpu::interrupt_enable();
}

void cv::broadcast() {
    cpu::interrupt_disable();

    while (!impl_ptr->waitQueue.empty()) {
        TcbPtr &threadToRun = impl_ptr->waitQueue.front();
        readyQueue.push(std::move(threadToRun));
        impl_ptr->waitQueue.pop();
    }
    
    cpu::interrupt_enable();
}
