#include <cstdio>
#include <ucontext.h>
#include "cpu.h"
#include "types.h"

void cpu::init(thread_startfunc_t body, void *arg) {
    // create temp tcb object and initialize it
    Tcb threadToRun;
    char *stack = new char[STACK_SIZE];
    threadToRun.ctx = std::unique_ptr<ucontext_t>(new ucontext_t());
    threadToRun.ctx->uc_stack.ss_sp = stack;
    threadToRun.ctx->uc_stack.ss_size = STACK_SIZE;
    threadToRun.ctx->uc_stack.ss_flags = 0;
    threadToRun.ctx->uc_link = nullptr;

    // switch invariant
    assert_interrupts_disabled();

    makecontext(threadToRun.ctx.get(), (void (*)()) os_wrapper, 2, body, arg);
    setcontext(threadToRun.ctx.get());
}
