#include <cassert>
#include "cv.h"
#include "types.h"

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
    m.unlock();
    
    assert(runningList.find(cpu::self()) != runningList.end());
    TcbPtr &currThread = runningList[cpu::self()];
    impl_ptr->waitQueue.push(std::move(currThread));

    switch_to_next_or_suspend(&impl_ptr->waitQueue.back()->ctx);
    m.lock();
}

void cv::signal() {

}

void cv::broadcast() {

}
