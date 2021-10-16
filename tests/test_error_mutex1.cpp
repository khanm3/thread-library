#include <iostream>
#include <stdexcept>
#include "thread.h"

void goodbye(void*);
void hello(void *);

mutex m;

void goodbye(void* a)
{
    m.lock();
    std::cout << "locked once" << (intptr_t)a << "\n";
    m.unlock();
    std::cout << "unlocked once" << (intptr_t)a << "\n";
    try {
        m.unlock();
    }
    catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        assert_interrupts_enabled();
        return;
    }
    std::cout << "unlocked twice" << (intptr_t)a << "\n"; // error should be thrown before this prints
}

void hello(void *a)
{
    std::cout << "Hello, world!" << std::endl;

    intptr_t one = 1;
    thread t1((thread_startfunc_t) goodbye, (void *) one);
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) hello, (void *) 100, false, false, 0);
    std::cout << "finishing boot" << std::endl;
}