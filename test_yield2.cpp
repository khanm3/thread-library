#include <iostream>
#include "thread.h"

// TEST YIELD 2
// tests yielding to self once, and then twice

void goodbye(void *a) {
    std::cout << "in goodbye before yield 1" << std::endl;

    thread::yield();

    std::cout << "in goodbye after yield 1 but before yield 2" << std::endl;

    thread::yield();

    std::cout << "in goodbye after yield 2" << std::endl;
}

void hello(void *a) {
    std::cout << "in hello before yield" << std::endl;

    thread::yield();

    std::cout << "in hello after yield" << std::endl;

    thread t2((thread_startfunc_t) goodbye, (void *) 0);
}

int main() {
    cpu::boot(1, (thread_startfunc_t) hello, (void *) 0, false, false, 0);
}
