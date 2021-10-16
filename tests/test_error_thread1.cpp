#include <iostream>
#include "types.h"
#include "thread.h"


void f1() {
    intptr_t i = 0;
    while(true) {
    try {
        thread t1 ( (thread_startfunc_t) f2, (void *) i);
    }
    catch(std::bad_alloc &ba){
        std::cout << "bad_alloc on thread "<< i << std::endl;
        assert_interrupts_enabled();
        return;
    }
    ++i;
    }
}

void f2(void * i) {
    std::cout << "Thread " << intptr_t(i) << "\n";
}

int main(int argc, char *argv[]) {
    cpu::boot(1, (thread_startfunc_t) f1, (void *) 0, false, false, 0);
    return 0;
}
