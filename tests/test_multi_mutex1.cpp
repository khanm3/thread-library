
#include <iostream>
#include "thread.h"

mutex m;

void func(void *a)
{
    intptr_t i = (intptr_t) a;
    std::cout << i << " pre-lock" << std::endl;
    m.lock();
    std::cout << i << " critical" << std::endl;
    if (i == 0) {
        thread((thread_startfunc_t) func, (void *) 1);
    }
    bool temp;
    for (int n = 0; n < 1000000; ++n) {
        temp = !temp;
    }
    m.unlock();
    for (int n = 0; n < 1000000; ++n) {
        temp = !temp;
    }
    std::cout << i << " end" << std::endl;
}

int main()
{
    cpu::boot(2, (thread_startfunc_t) func, (void *) 0, false, false, 0);
}
