#include <iostream>
#include "thread.h"

// t0 (func1) should remain running on CPU 0 for entire execution
// t1 (func2) should be suspended and awoken on CPU 1 twice before exiting

mutex m;

void busyWait() {
    bool temp = true;
    for (int n = 0; n < 1000000; ++n) {
        temp = !temp;
    }
}

void func2(void *a) {
    intptr_t i = (intptr_t) a;
    std::cout << i << " pre-lock 1" << std::endl;
    m.lock();
    std::cout << i << " critical 1" << std::endl;
    m.unlock();
    busyWait(); // wait for t0 to acquire lock
    std::cout << i << " pre-lock 2" << std::endl;
    m.lock();
    std::cout << i << " critical 2" << std::endl;
    m.unlock();
    busyWait(); // wait for t0 to exit
    std::cout << i << " exit" << std::endl;
}

void func1(void *a) {
    intptr_t i = (intptr_t) a;
    std::cout << i << " pre-lock 1" << std::endl;
    m.lock();
    thread((thread_startfunc_t) func2, (void *) 1);
    busyWait(); // wait for t1 to put itself on lock queue
    std::cout << i << " critical 1" << std::endl;
    m.unlock();
    busyWait(); // wait for t1 to enter critical section
    std::cout << i << " pre-lock 2" << std::endl;
    m.lock();
    busyWait(); // wait for t1 to put itself on lock queue
    std::cout << i << " critical 2" << std::endl;
    m.unlock();
    busyWait(); // wait for t1 to enter critical section
    std::cout << i << " exit" << std::endl;
}

int main()
{
    cpu::boot(2, (thread_startfunc_t) func1, (void *) 0, false, false, 0);
}
