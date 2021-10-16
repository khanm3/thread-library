#include <iostream>
#include "thread.h"

using namespace std;

// TEST JOIN 2
// test join with many threads on join queue

struct ParentArgs {
    intptr_t n;
    thread *child_ptr;
};

mutex m;

void child_func(void *a) {
    char *n = (char *) a;
    cout << n << " pre-yield" << endl;
    thread::yield();
    cout << n << " end" << endl;
}

void parent_func(void *a) {
    ParentArgs *args = (ParentArgs *) a;
    cout << args->n << " pre-join" << endl;
    args->child_ptr->join();
    cout << args->n << " end" << endl;
}

void start(void *a) {
    thread *child = new thread((thread_startfunc_t) child_func, (void *) "child");
    ParentArgs *args0 = new ParentArgs{0, child};
    thread t0((thread_startfunc_t) parent_func, (void *) args0);
    ParentArgs *args1 = new ParentArgs{1, child};
    thread t1((thread_startfunc_t) parent_func, (void *) args1);
    ParentArgs *args2 = new ParentArgs{2, child};
    thread t2((thread_startfunc_t) parent_func, (void *) args2);
}

int main() {
    cpu::boot(1, (thread_startfunc_t) start, (void *) 0, false, false, 0);
}
