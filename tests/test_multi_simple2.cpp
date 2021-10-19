
#include <iostream>
#include "thread.h"

mutex m;

void start(void *a)
{
    intptr_t i = (intptr_t) a;
    std::cout << i << " locking once" << std::endl;
    m.lock();
    std::cout << i << " locking twice" << std::endl;
    m.lock();
}

int main()
{
    cpu::boot(4, (thread_startfunc_t) start, (void *) 0, false, false, 0);
}
