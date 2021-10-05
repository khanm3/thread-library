#include <iostream>
#include "thread.h"

void goodbye(void*);
void hello(void *);

void goodbye(void* a)
{
    std::cout << "Goodbye, cruel world!\n" << "From thread: " << (intptr_t)a << "\n";
    // thread t1((thread_startfunc_t) hello, a);
}

void hello(void *a)
{
    std::cout << "Hello, world!" << std::endl;

    intptr_t one = 1;
    intptr_t two = 2;
    intptr_t three = 3;
    thread t1((thread_startfunc_t) goodbye, (void *) one);
    thread t2((thread_startfunc_t) goodbye, (void *) two);
    thread t3((thread_startfunc_t) goodbye, (void *) three);
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) hello, (void *) 100, false, false, 0);
    std::cout << "finishing boot" << std::endl;
}
