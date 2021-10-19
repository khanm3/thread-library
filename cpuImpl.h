
#ifndef _CPUIMPL_H
#define _CPUIMPL_H

#include <ucontext.h>
#include "cpu.h"

enum CpuState {
    CPU_INITIALIZED,
    CPU_RUNNING,
    CPU_SUSPENDED,
};

class cpu::impl {
public:
    CpuState state;
    ucontext_t *suspendCtx;

    impl();
};

#endif