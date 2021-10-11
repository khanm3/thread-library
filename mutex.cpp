#include "mutex.h"
#include "types.h"

class mutex::impl {
    public:
        bool free;
        std::queue<TcbPtr> lockQueue;
};

mutex::mutex() {
    cpu::interrupt_disable(); // do we need to disable interrupts
    impl_ptr = new impl();
    impl_ptr->free = true;
    impl_ptr->lockQueue = std::queue<TcbPtr>();
    cpu::interrupt_enable();
}

mutex::~mutex() {
    delete impl_ptr;
}

void mutex::lock() {
    cpu::interrupt_disable();
    if(!free) {
        TcbPtr &currThread = runningList[cpu::self()];
        *currThread->state = BLOCKED;
        impl_ptr->lockQueue.push(std::move(currThread));
        switch_to_next_or_suspend(&impl_ptr->lockQueue.back()->ctx);
    } else {
        impl_ptr->free = false;
    }
    cpu::interrupt_enable();
}

void mutex::unlock() {
    cpu::interrupt_disable();
    impl_ptr->free = true;
    if(!impl_ptr->lockQueue.empty()) {
        TcbPtr &threadToLock = impl_ptr->lockQueue.front();
        *threadToLock->state = READY;
        readyQueue.push(std::move(threadToLock));
        impl_ptr->lockQueue.pop();
        impl_ptr->free = false;
    }
    cpu::interrupt_enable();
}