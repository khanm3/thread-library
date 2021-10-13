#include "mutex.h"
#include "types.h"

class mutex::impl {
    public:
        std::queue<TcbPtr> lockQueue;
        Tcb *owner;

        void unlockHelper() {
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

        void lockHelper()  {
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
};