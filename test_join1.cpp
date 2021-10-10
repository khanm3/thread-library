#include <iostream>
#include "thread.h"

// TEST JOIN 1
// tests thread::join
// one parent thread, two child threads
// one call to join on first child thread
// create second child thread after returing from join on first child thread

void child(void* a)
{
    std::cout << "in child " << (intptr_t) a << std::endl;
}

void parent(void *a)
{
    std::cout << "starting parent" << std::endl;

    thread t1((thread_startfunc_t) child, (void *) 1);

    std::cout << "in parent before join on child 1" << std::endl;
    t1.join();
    std::cout << "in parent after join on child 1" << std::endl;

    thread t2((thread_startfunc_t) child, (void *) 2);

    // std::cout << "in parent before join on child 2" << std::endl;
    // t2.join();
    // std::cout << "in parent before join on child 2" << std::endl;

    // thread t3((thread_startfunc_t) child, (void *) 3);

    std::cout << "exiting parent" << std::endl;
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 0, false, false, 0);
}