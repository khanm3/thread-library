// Signal without holding a mutex
#include <iostream>
#include <stdexcept>
#include "thread.h"

void f2(void*);
void f1(void *);

mutex m1;
mutex m2;
cv c;

void f2(void* a)
{
    std::cout << "Signaling a a cv without holding the mutex\n";
    c.signal();
    m2.lock();
    std::cout << "Waiting on a control variable with 2 different mutexes\n";
    c.wait(m2);
}

void f1(void *a)
{
    std::cout << "Hello, world!" << std::endl;
    m1.lock();
    intptr_t one = 1;
    thread t1((thread_startfunc_t) f2, (void *) one);
    c.wait(m1);
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) f1, (void *) 100, false, false, 0);
}
