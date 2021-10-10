#include <iostream>
#include "thread.h"

// TEST JOIN 3
// tests thread::join
// one parent thread, one child thread
// one call to join
// child finishes after call to join

void child(void *a) {
    // 5. OUTPUT LINE #2
    std::cout << "in child" << std::endl;

    // 6. end, move parent from join queue to ready queue, switch to parent
}

void parent(void *a) {
    // 2. construct child and put it on ready queue
    thread t1((thread_startfunc_t) child, (void *) 0);

    // 3. OUTPUT LINE #1
    std::cout << "in parent before join" << std::endl;

    // 4. block parent and switch to child
    t1.join();
    // 7. resume

    // 8. OUTPUT LINE #3
    std::cout << "in parent after join" << std::endl;
}

int main() {
    // 1. boot
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 0, false, false, 0);
}
