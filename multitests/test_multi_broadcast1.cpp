
#include <iostream>
#include "thread.h"

mutex m;
cv c;
int k = 0;

void func2(void *a) {
    intptr_t i = (intptr_t) a;
    std::cout << i << " pre-lock" << std::endl;
    m.lock();
    while (k != 1) {
        std::cout << i << " waiting until k = 1" << std::endl;
        c.wait(m);
    }
    std::cout << i << " critical" << std::endl;
    m.unlock();
    std::cout << i << " end" << std::endl;
}

void func1(void *a) {
    intptr_t i = (intptr_t) a;
    for (intptr_t n = 1; n < 4; ++n) {
        thread((thread_startfunc_t) func2, (void *) n);
    }
    bool temp;
    for (int n = 0; n < 1000000; ++n) {
        temp = !temp;
    }
    std::cout << i << " pre-lock" << std::endl;
    m.lock();
    k = 1;
    std::cout << i << " set k to 1" << std::endl;
    c.broadcast();
    m.unlock();
    for (int n = 0; n < 1000000; ++n) {
        temp = !temp;
    }
    std::cout << i << " end" << std::endl;
}

int main()
{
    cpu::boot(4, (thread_startfunc_t) func1, (void *) 0, false, false, 0);
}
