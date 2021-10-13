#include <cassert>
#include "mutex.h"
#include "types.h"

class mutex::impl {
    public:
        std::queue<TcbPtr> lockQueue;
        Tcb *owner;
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

    TcbPtr &currThread = runningList[cpu::self()];

    if (impl_ptr->owner) {
        // if lock is not free, add thread to lock's waiting queue and switch
        *currThread->state = BLOCKED;
        impl_ptr->lockQueue.push(std::move(currThread));
        switch_to_next_or_suspend(&impl_ptr->lockQueue.back()->ctx);
    } else {
        // else, acquire the lock
        impl_ptr->owner = currThread.get();
    }
    cpu::interrupt_enable();
}

void mutex::unlock() {
    cpu::interrupt_disable();

    Tcb *currThread = runningList[cpu::self()].get();
    if (currThread != impl_ptr->owner) {
        throw std::runtime_error("error: thread tried to unlock mutex not belonging to it");
    }

    impl_ptr->owner = nullptr;

    if(!impl_ptr->lockQueue.empty()) {
        TcbPtr &threadToLock = impl_ptr->lockQueue.front();
        *threadToLock->state = READY;
        readyQueue.push(std::move(threadToLock));
        impl_ptr->lockQueue.pop();
        impl_ptr->owner = readyQueue.back().get();
    }
    cpu::interrupt_enable();
}