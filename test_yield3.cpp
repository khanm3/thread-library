#include <iostream>
#include "thread.h"

// TEST YIELD 3
// three threads yielding to each other
// 1. CPU: 1, Ready Queue: [_, _]
// 2. CPU: 1, Ready Queue: [2, _]
// 3. CPU: 2, Ready Queue: [1, _]
// 4. CPU: 2, Ready Queue: [1, 3]
// 5. CPU: 1, Ready Queue: [3, 2]
// 6. CPU: 3, Ready Queue: [2, 1]
// 7. CPU: 2, Ready Queue: [1, _]
// 8. CPU: 1, Ready Queue: [_, _]

void f3(void *a) {
    std::cout << "6" << std::endl;
}

void f2(void *a) {
    std::cout << "3" << std::endl;

    thread t3((thread_startfunc_t) f3, (void *) 0);

    std::cout << "4" << std::endl;

    thread::yield();

    std::cout << "7" << std::endl;
}

void f1(void *a) {
    std::cout << "1" << std::endl;

    thread t2((thread_startfunc_t) f2, (void *) 0);

    std::cout << "2" << std::endl;

    thread::yield();

    std::cout << "5" << std::endl;

    thread::yield();

    std::cout << "8" << std::endl;
}

int main() {
    cpu::boot(1, (thread_startfunc_t) f1, (void *) 0, false, false, 0);
}
