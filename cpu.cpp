#include <cstdio>
#include "thread.h"

void cpu::init(thread_startfunc_t body, void *arg) {
    std::printf("Starting cpu::init...\n");
    body(arg);
    std::printf("Finishing cpu::init...\n");
}
