#include <iostream>
#include <stdexcept>
#include "thread.h"

void goodbye(void*);
void hello(void *);
void waitError(void *);

mutex m;
cv c;

void waitError(void* a)
{
    std::cout << "Thread " << (intptr_t) a << " calls wait with unacquired mutex\n";
    try 
    {
        while (true) {
            c.wait(m);
        }
    }
    catch (std::runtime_error &e) 
    {
        std::cout << e.what() << std::endl;
        assert_interrupts_enabled();
        return;
    }
}

void goodbye(void* a)
{
    m.lock();
    std::cout << "Thread " << (intptr_t) a << " has acquired mutex\n";
    intptr_t two = 2;
    thread t1((thread_startfunc_t) waitError, (void *) two);
    thread::yield();
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