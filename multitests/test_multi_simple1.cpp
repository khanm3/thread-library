
#include <iostream>
#include "thread.h"

using namespace std;

void f(void* a)
{
    cout << (intptr_t) a << " start" << endl;
    cout << (intptr_t) a << " end" << endl;
}

void start(void *a)
{
    cout << (intptr_t) a << " start" << endl;
    thread t1((thread_startfunc_t) f, (void *) 1);
    thread t2((thread_startfunc_t) f, (void *) 2);
    thread t3((thread_startfunc_t) f, (void *) 3);
    cout << (intptr_t) a << " end" << endl;
}

int main()
{
    cpu::boot(4, (thread_startfunc_t) start, (void *) 0, false, false, 0);
}
