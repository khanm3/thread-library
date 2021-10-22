#include <iostream>
#include <stdexcept>
#include "thread.h"

void f2(void *i) {
    // do nothing
}

void f1(void * a) {
    while (true) {
        try {
            thread((thread_startfunc_t) f2, (void *) 0);
        }
        catch(std::bad_alloc &ba){
            std::cout << "error: bad_alloc on thread creation" << std::endl;
            assert_interrupts_enabled();
            return;
        }
    }
}

int main() {
    cpu::boot(1, (thread_startfunc_t) f1, (void *) 0, false, false, 0);
}
