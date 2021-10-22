#include "mutexImpl.h"

mutex::impl::impl()
    : ownerState(ThreadStatePtr(nullptr))
{

}

void mutex::impl::lockHelper()  {
    assert_interrupts_disabled();

    TcbPtr &currThread = runningList[cpu::self()];

    // if lock is held, block current thread
    if (ownerState) {
        // move current thread to lock's wait queue
        *(currThread->state) = BLOCKED;
        lockQueue.push(std::move(currThread));

        // switch to next ready thread if there is one, else suspend
        switch_to_next_or_suspend(&lockQueue.back()->ctx);

        // switch invariant - assert interrupts disabled after returning from switch
        assert_interrupts_disabled();

        // if there are any threads on the finished list, clean them up
        cleanup_finished_list();
    }
    // else lock is free, acquire it
    else {
        // acquire the lock
        ownerState = currThread->state;
    }
}

void mutex::impl::unlockHelper() {
    assert_interrupts_disabled();

    Tcb *currThread = runningList[cpu::self()].get();

    // throw an error if the calling thread does not hold the lock
    if (currThread->state != ownerState) {
        throw std::runtime_error("error: thread tried to unlock mutex not belonging to it");
    }

    // otherwise, the current thread owns the lock, we can safely release it
    ownerState.reset();

    // if there's a thread on the lock queue, hand the lock off to it
    if (!lockQueue.empty()) {
        // move the next thread on the lock queue to the ready queue
        *(lockQueue.front()->state) = READY;
        readyQueue.push(std::move(lockQueue.front()));
        lockQueue.pop();

        // make the lock held again, this time by the thread the lock is being
        // handed off to
        ownerState = readyQueue.back()->state;

        // send IPI
        send_ipi(1);
    }
}
