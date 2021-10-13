#include <cassert>
#include "mutex.h"
#include "types.h"
#include "mutexImpl.h"

mutex::mutex() {
    cpu::interrupt_disable(); // do we need to disable interrupts
    impl_ptr = new impl();
    impl_ptr->lockQueue = std::queue<TcbPtr>();
    impl_ptr->owner = nullptr;
    cpu::interrupt_enable();
}

mutex::~mutex() {
    delete impl_ptr;
}

void mutex::lock() {
    cpu::interrupt_disable();
    impl_ptr->lockHelper();
    cpu::interrupt_enable();
}

void mutex::unlock() {
    cpu::interrupt_disable();
    impl_ptr->unlockHelper();
    cpu::interrupt_enable();
}