#include <iostream>
#include "thread.h"

// TEST JOIN 4
// tests thread::join
// one parent thread, one child thread
// one call to join
// child finishes before call to join

void child(void *a) {
    // 4. OUTPUT LINE #1
    std::cout << "in child" << std::endl;

    // 5. end, switch to parent
}

void parent(void *a) {
    // 2. construct child and put it on ready queue
    thread t1((thread_startfunc_t) child, (void *) 0);

    // 3. preempt parent onto ready queue, switch to child
    thread::yield();
    // 6. resume

    // 7. OUTPUT LINE #2
    std::cout << "in parent before join" << std::endl;

    // 8. call to join returns immediately
    t1.join();

    // 9. OUTPUT LINE #3
    std::cout << "in parent after join" << std::endl;
}

int main() {
    // 1. boot
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 0, false, false, 0);
}
