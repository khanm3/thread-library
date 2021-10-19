
#include <iostream>
#include "thread.h"

void occupy_cpu2(void *a) {
    intptr_t i = (intptr_t) a;
    bool temp;
    for (int n = 0; n < 1000000; ++n) {
        temp = !temp;
    }
    std::cout << i << " begin + end" << std::endl;
}

void child_func(void *a) {
    intptr_t i = (intptr_t) a;
    bool temp;
    for (int n = 0; n < 1000000; ++n) {
        temp = !temp;
    }
    std::cout << i << " begin + end" << std::endl;
}

void parent_func(void *a) {
    intptr_t i = (intptr_t) a;
    std::cout << i << " pre-join" << std::endl;
    thread((thread_startfunc_t) occupy_cpu2, (void *) 2);
    thread child((thread_startfunc_t) child_func, (void *) 1);
    child.join();
    std::cout << i << " end" << std::endl;
}

int main()
{
    cpu::boot(2, (thread_startfunc_t) parent_func, (void *) 0, false, false, 0);
}
