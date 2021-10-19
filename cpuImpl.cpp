
#include "cpuImpl.h"
#include "types.h"

cpu::impl::impl()
    : state(CPU_INITIALIZED)
    , suspendCtx(new ucontext_t)
{
    suspendCtx->uc_stack.ss_sp = new char[STACK_SIZE];
    suspendCtx->uc_stack.ss_size = STACK_SIZE;
    suspendCtx->uc_stack.ss_flags = 0;
    suspendCtx->uc_link = nullptr;
    makecontext(suspendCtx, (void (*)()) os_suspend, 0);
}
