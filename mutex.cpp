#include <cassert>
#include "mutex.h"
#include "mutexImpl.h"
#include "types.h"

mutex::mutex() {
    cpu::interrupt_disable(); // do we need to disable interrupts
    impl_ptr = new impl();
    cpu::interrupt_enable();
}

mutex::~mutex() {
    delete impl_ptr;
}

void mutex::lock() {
    cpu::interrupt_disable();
    // TODO: MULTIPROCESSOR - acquire guard

    impl_ptr->lockHelper();

    // TODO: MULTIPROCESSOR - release guard
    cpu::interrupt_enable();
}

void mutex::unlock() {
    cpu::interrupt_disable();
    // TODO: MULTIPROCESSOR - acquire guard

    impl_ptr->unlockHelper();

    // TODO: MULTIPROCESSOR - release guard
    cpu::interrupt_enable();
}
