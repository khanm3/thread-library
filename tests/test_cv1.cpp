#include <iostream>
#include "thread.h"

mutex m1;
cv cv1;
int i = 0;

void b(void *a) {
    m1.lock();

    std::cout << "in thread b, incrementing i from 0 to 1" << std::endl;
    ++i;

    std::cout << "in thread b, signalling a that i != 0" << std::endl;
    cv1.signal();

    m1.unlock();
}

void a(void *a) {
    m1.lock();

    std::cout << "in thread a, i starting at 0" << std::endl;
    thread tb((thread_startfunc_t) b, (void *) 0);

    while (i == 0) {
        std::cout << "in thread a, waiting while i is equal to 0" << std::endl;
        cv1.wait(m1);
    }

    std::cout << "in thread a, i is equal to 1" << std::endl;

    m1.unlock();
}

int main() {
    cpu::boot(1, (thread_startfunc_t) a, (void *) 0, false, false, 0);
}
