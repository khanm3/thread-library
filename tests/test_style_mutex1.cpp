#include <iostream>
#include <stdexcept>
#include "thread.h"

mutex m;

void func0(void *a) {
    intptr_t i = (intptr_t) a;
    std::cout << i << " pre-lock" << std::endl;
    m.lock();
    std::cout << i << " exit while holding lock" << std::endl;
}

void func1(void *a) {
    intptr_t i = (intptr_t) a;
    std::cout << i << " attempting to unlock" << std::endl;
    m.unlock();
    std::cout << i << " exit after unlock" << std::endl;
}

void start_func(void *a) {
    if (true) {
        thread t0((thread_startfunc_t) func0, (void *) 0);
        thread::yield();
    }

    try {
        thread t1((thread_startfunc_t) func1, (void *) 1);
    }
    catch (const std::runtime_error &re) {
        std::cout << "runtime error: 1 tried to free a lock that 0 holds" << std::endl;
        assert_interrupts_enabled();
        return;
    }
}

int main() {
    cpu::boot(1, (thread_startfunc_t) start_func, (void *) 0, false, false, 0);
}
