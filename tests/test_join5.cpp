#include <iostream>
#include "thread.h"

// TEST JOIN 5
// tests thread::join
// one parent thread, two child threads
// one call to join each, two in total
// child threads finish after calls to join

void child(void *a) {
    std::cout << "in child " << (intptr_t) a << std::endl;
}

void parent(void *a) {
    intptr_t i = (intptr_t) a;

    // child thread 1 
    thread t1((thread_startfunc_t) child, (void *) 1);
    std::cout << "in parent " << i << " before join 1" << std::endl;
    t1.join();
    std::cout << "in parent " << i << " after join 1" << std::endl;

    // child thread 2
    thread t2((thread_startfunc_t) child, (void *) 2);
    std::cout << "in parent " << i << " before join 2" << std::endl;
    t2.join();
    std::cout << "in parent " << i << " after join 2" << std::endl;
}

int main() {
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 0, false, false, 0);
}
