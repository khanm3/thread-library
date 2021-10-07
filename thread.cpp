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
	cpu::interrupt_disable();
    // TODO: MULTIPROCESSOR - switch invariant - acquire guard
    impl_ptr = new impl();

    // create tcb and put it onto ready queue
    printf("Hello 1\n");
    readyQueue.push(TcbPtr(new Tcb(READY, body, arg)));
    printf("Hello 2\n");
    impl_ptr->state = readyQueue.back()->state;
    printf("Hello 3\n");
    impl_ptr->joinQueue = readyQueue.back()->joinQueue;
    printf("Hello 4\n");
    // TODO: MULTIPROCESSOR - IPI to available CPU
    // TODO: MULTIPROCESSOR - free guard
    cpu::interrupt_enable();
}

thread::~thread() = default;

void thread::yield() {
    cpu::interrupt_disable();
    // TODO: MULTIPROCESSOR - acquire guard

    if (!readyQueue.empty()) {
        // put current thread on ready queue
        assert(runningList.find(cpu::self()) != runningList.end());
        TcbPtr &currThread = runningList[cpu::self()];
        *(currThread->state) = READY;
        readyQueue.push(std::move(currThread));

        // put next thread in ready queue onto running list
        *(readyQueue.front()->state) = RUNNING;
        currThread = std::move(readyQueue.front());
        readyQueue.pop();

        // switch to next thread
        swapcontext(&readyQueue.back()->ctx, &currThread->ctx);
    }

    // TODO: MULTIPROCESSOR - free guard
    cpu::interrupt_enable();
}
