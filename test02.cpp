#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

int g = 0;

void loop(void *a)
{
    char *id = (char *) a;
    int i;

    cout << "loop called with id " << id << endl;

    for (i=0; i<5; i++, g++) {
        cout << id << ":\t" << i << "\t" << g << endl;
        thread::yield();
    }

    cout << id << ":\t" << i << "\t" << g << endl;
}

void parent(void *a)
{
    intptr_t arg = (intptr_t) a;

    cout << "parent called with arg " << arg << endl;

    thread t1 ( (thread_startfunc_t) loop, (void *) "child thread");

    loop( (void *) "parent thread");
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}
