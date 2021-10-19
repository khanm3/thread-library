#ifndef _TYPES_H
#define _TYPES_H

#include <map>
#include <memory>
#include <queue>
#include <ucontext.h>
#include "thread.h"

enum ThreadState {
    INITIALIZED,
    RUNNING,
    FINISHED,
    BLOCKED,
    READY,
};

class Tcb {
public:
    // EFFECTS: creates an empty TCB. it contains no context or stack, and
    // its initial state is INITIALIZED.
    Tcb();

    // EFFECTS: creates a valid TCB with an allocated context and stack
    // whose state is initialized to the given state, and whose program
    // counter is set to the given function with the given argument
    Tcb(ThreadState, thread_startfunc_t, void *);

    // use default move constructors
    Tcb(Tcb&&) = default;
    Tcb& operator=(Tcb&&) = default;

    // disable copy constructors
    Tcb(const Tcb&) = delete;
    Tcb& operator=(const Tcb&) = delete;

    // REQUIRES: this tcb manages a stack (as well as a context, implicitly)
    // EFFECTS: frees the stack managed by this tcb
    void freeStack();

    // MEMBER VARIABLES //
    ucontext_t ctx;
    char *stackPtr;
    std::shared_ptr<ThreadState> state;
    std::shared_ptr<std::queue<std::unique_ptr<Tcb>>> joinQueue;
};

class RaiiLock {
public:
    RaiiLock();
    ~RaiiLock();

    static void lock();
    static void unlock();
    static void acquireGuard();
    static void releaseGuard();
};

using TcbPtr = std::unique_ptr<Tcb>;
using ThreadStatePtr = std::shared_ptr<ThreadState>;
using JoinQueuePtr = std::shared_ptr<std::queue<TcbPtr>>;

extern std::queue<TcbPtr> readyQueue;
extern std::vector<TcbPtr> finishedList;
extern std::map<cpu *, TcbPtr> runningList;

void os_wrapper(thread_startfunc_t, void *);

void switch_to_next_or_suspend(ucontext_t *);

void cleanup_finished_list();

void yield_helper();

void handle_timer();

void os_suspend();

#endif
