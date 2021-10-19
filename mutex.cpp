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
    impl_ptr->lockHelper();
}

void mutex::unlock() {
	RaiiLock l;
    impl_ptr->unlockHelper();
}
