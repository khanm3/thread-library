#include <iostream>
#include "thread.h"

void goodbye(void*);
void hello(void *);

void goodbye(void* a)
{
    std::cout << "Goodbye! From thread: " << (intptr_t)a << "\n";
}

void hello(void *a)
{
    std::cout << "Hello, world!" << std::endl;

    intptr_t one = 1;
    thread t1((thread_startfunc_t) goodbye, (void *) one);
    t1.join();
    t1.join(); //t1 is guaranteed to have finished by here
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) hello, (void *) 100, false, false, 0);
    std::cout << "finishing boot" << std::endl;
}