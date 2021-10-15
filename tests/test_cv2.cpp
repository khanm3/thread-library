#include <iostream>
#include "thread.h"

mutex m1;
cv cv1;
int i = 0;

void f2(void *a) {
    intptr_t n = (intptr_t) a;

    m1.lock();

    while (i != n) {
        std::cout << "i = " << i << ", waiting in thread " << n << std::endl;
        cv1.wait(m1);
    }

    std::cout << "i = " << i << ", in critical section in thread " << n << std::endl;
    ++i;
    std::cout << "i = " << i << ", in critical section in thread " << n << std::endl;

    cv1.broadcast();

    m1.unlock();
}

void f1(void *a) {
    m1.lock();

    for (intptr_t n = 9; n >= 0; --n) {
        thread t((thread_startfunc_t) f2, (void *) n);
    }

    m1.unlock();
}

int main() {
    cpu::boot(1, (thread_startfunc_t) f1, (void *) 0, false, false, 0);
}
