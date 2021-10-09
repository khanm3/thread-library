#include <iostream>
#include "thread.h"

void goodbye(void*);
void hello(void *);

void printFromJoin(void* a)
{
    thread::yield();
    std::cout << "Goodbye! From thread: " << (intptr_t)a << "\n";
}

void goodbye(void* a)
{
    std::cout << "Goodbye! From thread: " << (intptr_t)a << "\n";
}

void hello(void *a)
{
    std::cout << "Hello, world!" << std::endl;

    intptr_t one = 1;
    intptr_t two = 2;
    intptr_t three = 3;
    thread t1((thread_startfunc_t) goodbye, (void *) one);
    t1.join();
    thread t2((thread_startfunc_t) printFromJoin, (void *) two);
    //t2.join();
    thread t3((thread_startfunc_t) goodbye, (void *) three);
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) hello, (void *) 100, false, false, 0);
    std::cout << "finishing boot" << std::endl;
}