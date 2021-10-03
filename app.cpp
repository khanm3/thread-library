#include <iostream>
#include "thread.h"

void hello(void *a)
{
    std::cout << "Hello, world!" << std::endl;
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) hello, (void *) 100, false, false, 0);
    std::cout << "finishing boot" << std::endl;
}
