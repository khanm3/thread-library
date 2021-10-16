#include <iostream>
#include <cstring>
#include "thread.h"

mutex m;

void locker(void *a) {
    std::cout << "Thread: " << (char *) a << " locking\n";
    m.lock();

    if (!strcmp((char*)a, "1")) {
        std::cout << "Thread: " << (char *) a << " yielding\n";
        thread::yield();
        thread t3 ( (thread_startfunc_t) locker, (void *) "3");
    }

    std::cout << "Thread: " << (char *) a << " unlocking\n";
    m.unlock();
    
    std::cout << "Thread: " << (char *) a << " finishing\n";
}

void parent(void *a) {
    thread t1 ( (thread_startfunc_t) locker, (void *) "1");
    thread t2 ( (thread_startfunc_t) locker, (void *) "2");
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}