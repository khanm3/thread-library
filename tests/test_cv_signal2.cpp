#include <iostream>
#include "thread.h"

using namespace std;

// TEST CV SIGNAL 2
// test signal with 1 thread on wait queue

mutex m1;
cv cv1;
int i = 0;

void f(void *a) {
    intptr_t n = (intptr_t) a;
    cout << n << " pre-lock" << endl;
    m1.lock();
    // thread 0 has no wait condition, so run immediately
    if (n == 0) {
        // ...
    }
    // threads 1 to infinity wait until i > 0
    else {
        while (i <= 0) {
            cout << n << " waiting" << endl;
            cv1.wait(m1);
        }
    }
    cout << n << " critical" << endl;
    ++i;
    cv1.signal();
    m1.unlock();
    cout << n << " end" << endl;
}

void start(void *a) {
    thread t1((thread_startfunc_t) f, (void *) 1);
    thread t0((thread_startfunc_t) f, (void *) 0);
}

int main() {
    cpu::boot(1, (thread_startfunc_t) start, (void *) 0, false, false, 0);
}
