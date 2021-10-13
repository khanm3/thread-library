#include <cassert>
#include "mutex.h"
#include "types.h"

class mutex::impl {
    public:
        std::queue<TcbPtr> lockQueue;
        Tcb *owner;

        void unlockHelper();
        void lockHelper();
};

mutex::mutex() {
    cpu::interrupt_disable(); // do we need to disable interrupts
    impl_ptr = new impl();
    impl_ptr->lockQueue = std::queue<TcbPtr>();
    impl_ptr->owner = nullptr;
    cpu::interrupt_enable();
}

mutex::~mutex() {
    delete impl_ptr;
}

void mutex::lock() {
    cpu::interrupt_disable();
    impl_ptr->lockHelper();
    cpu::interrupt_enable();
}

void mutex::unlock() {
    cpu::interrupt_disable();
    impl_ptr->unlockHelper();
    cpu::interrupt_enable();
}

void mutex::impl::lockHelper() {
    TcbPtr &currThread = runningList[cpu::self()];

    if (owner) {
        // if lock is not free, add thread to lock's waiting queue and switch
        *currThread->state = BLOCKED;
        lockQueue.push(std::move(currThread));
        switch_to_next_or_suspend(&lockQueue.back()->ctx);
    } else {
        // else, acquire the lock
        owner = currThread.get();
    }
}

void mutex::impl::unlockHelper() {
    Tcb *currThread = runningList[cpu::self()].get();
    if (currThread != owner) {
        throw std::runtime_error("error: thread tried to unlock mutex not belonging to it");
    }

    owner = nullptr;

    if (!lockQueue.empty()) {
        TcbPtr &threadToLock = lockQueue.front();
        *threadToLock->state = READY;
        readyQueue.push(std::move(threadToLock));
        lockQueue.pop();
        owner = readyQueue.back().get();
    }
}
