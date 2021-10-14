#include <iostream>
#include "thread.h"

// TEST JOIN 2
// tests thread::join
// one parent thread, one child thread
// two calls to join on the same child thread object
// child thread finishes after first call to join

void child(void* a)
{
    std::cout << "in child" << std::endl;
}

void parent(void *a)
{
    thread t1((thread_startfunc_t) child, (void *) 1);

    std::cout << "in parent before first call to join" << std::endl;
    t1.join();
    std::cout << "in parent after first call to join and "
              << "before second call to join"
              << std::endl;
    t1.join(); //t1 is guaranteed to have finished by here
    std::cout << "in parent after second call to join" << std::endl;
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 0, false, false, 0);
    std::cout << "finishing boot" << std::endl;
}