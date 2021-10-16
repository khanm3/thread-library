#include <cassert>
#include "mutex.h"
#include "mutexImpl.h"
#include "types.h"

mutex::mutex() {
    impl_ptr = new impl();
}

mutex::~mutex() {
    delete impl_ptr;
}

void mutex::lock() {
	RaiiLock l;
    // TODO: MULTIPROCESSOR - acquire guard

    impl_ptr->lockHelper();

    // TODO: MULTIPROCESSOR - release guard
}

void mutex::unlock() {
	RaiiLock l;
    // TODO: MULTIPROCESSOR - acquire guard

    impl_ptr->unlockHelper();

    // TODO: MULTIPROCESSOR - release guard
}
