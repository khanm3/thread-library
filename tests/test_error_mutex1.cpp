#include <iostream>
#include <stdexcept>
#include "thread.h"

mutex m;

void hello(void *a) {
    try {
        m.unlock();
    }
    catch (std::runtime_error &e) {
        std::cout << "error: thread tried to unlock mutex not belonging to it" << std::endl;
        assert_interrupts_enabled();
        return;
    }
    std::cout << "unlocked" << std::endl; // error should be thrown before this prints
}

int main() {
    cpu::boot(1, (thread_startfunc_t) hello, (void *) 0, false, false, 0);
}
