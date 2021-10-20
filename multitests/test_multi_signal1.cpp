
#include <iostream>
#include "thread.h"

// if CPUs that are passed nullptr run the next ready thread if there is one
// and suspend otherwise, rather than only ever suspending, this test case will
// sometimes have both CPUs initialized to running in its debug output

mutex m;
cv c;
int k = 0;

void func2(void *a) {
    intptr_t i = (intptr_t) a;
    bool temp;
    for (int n = 0; n < 2000000; ++n) {
        temp = !temp;
    }
    std::cout << i << " pre-lock" << std::endl;
    m.lock();
    k = 1;
    std::cout << i << " set k to 1" << std::endl;
    c.signal();
    m.unlock();
    for (int n = 0; n < 1000000; ++n) {
        temp = !temp;
    }
    std::cout << i << " end" << std::endl;
}

void func1(void *a) {
    intptr_t i = (intptr_t) a;
    bool temp;
    std::cout << i << " pre-lock" << std::endl;
    m.lock();
    thread((thread_startfunc_t) func2, (void *) 1);
    for (int n = 0; n < 1000000; ++n) {
        temp = !temp;
    }
    while (k != 1) {
        std::cout << i << " waiting until k = 1" << std::endl;
        c.wait(m);
    }
    std::cout << i << " critical" << std::endl;
    m.unlock();
    std::cout << i << " end" << std::endl;
}

int main()
{
    cpu::boot(2, (thread_startfunc_t) func1, (void *) 0, false, false, 0);
}
