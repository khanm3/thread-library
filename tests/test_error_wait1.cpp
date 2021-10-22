#include <iostream>
#include <stdexcept>
#include "thread.h"

mutex m;
cv c;
int i = 0;

/*
void goodbye(void *a) {
    std::cout << (intptr_t) a << " pre-lock" << std::endl;
    m.lock();
    std::cout << (intptr_t) a << " critical" << std::endl;
    i = 1;
    c.signal();
    m.unlock();
    std::cout << (intptr_t) a << " exit" << std::endl;
}
*/

void hello(void *a) {
    // thread((thread_startfunc_t) goodbye, (void *) 1);
    while (i != 1) {
        try {
            c.wait(m);
        }
        catch (std::runtime_error &re) {
            std::cout << "error: thread tried to unlock mutex not belonging to it" << std::endl;
            assert_interrupts_enabled();
            return;
        }
    }
    std::cout << (intptr_t) a << " critical" << std::endl;
    m.unlock();
    std::cout << (intptr_t) a << " exit" << std::endl;
}

int main() {
    cpu::boot(1, (thread_startfunc_t) hello, (void *) 0, false, false, 0);
}
