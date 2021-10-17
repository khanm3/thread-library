#include <cstdint>
#include <iostream>
#include "thread.h"

using namespace std;

void hello(void *arg) {
    cout << "Hello " << (intptr_t) arg << endl;
}

void start(void *arg) {
    cout << "Start" << endl;

    thread t0((thread_startfunc_t) hello, (void *) 0);
    thread t1((thread_startfunc_t) hello, (void *) 1);
}

int main() {
    cpu::boot(2, (thread_startfunc_t) start, (void *) 0, false, false, 0);
}
