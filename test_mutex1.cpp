#include <iostream>
#include "thread.h"

void goodbye(void*);
void hello(void *);

mutex m;

void goodbye(void* a)
{
    m.lock();
    std::cout << "Goodbye! From thread: " << (intptr_t)a << "\n";
    m.unlock();
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