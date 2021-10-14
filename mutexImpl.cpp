#include "mutexImpl.h"

mutex::impl::impl()
    : owner(nullptr)
{

}

void mutex::impl::lockHelper()  {
    TcbPtr &currThread = runningList[cpu::self()];

    // lock is held
    if (owner) {
        // move current thread to lock's wait queue
        *(currThread->state) = BLOCKED;
        lockQueue.push(std::move(currThread));

        // switch to next ready thread if there is one, else suspend
        switch_to_next_or_suspend(&lockQueue.back()->ctx);

        // if there are any threads on the finished list, clean them up
        cleanup_finished_list();
    }
    // lock is free
    else {
        // acquire the lock
        owner = currThread.get();
    }
}

void mutex::impl::unlockHelper() {
    Tcb *currThread = runningList[cpu::self()].get();

    // check that the calling thread holds the lock
    if (currThread != owner) {
        throw std::runtime_error("error: thread tried to unlock mutex not belonging to it");
    }

    // release the lock
    owner = nullptr;

    // if there's a thread on the lock queue, hand the lock off to it
    if (!lockQueue.empty()) {
        // move the next thread on the lock queue to the ready queue
        *(lockQueue.front()->state) = READY;
        readyQueue.push(std::move(lockQueue.front()));
        lockQueue.pop();

        // make the lock held again, this time by the thread the lock is being
        // handed off to
        owner = readyQueue.back().get();
    }
}
