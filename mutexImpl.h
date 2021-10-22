#ifndef _MUTEXIMPL_H
#define _MUTEXIMPL_H

#include "mutex.h"
#include "types.h"

class mutex::impl {
public:
    uintptr_t owner;
    std::queue<TcbPtr> lockQueue;

    impl();
    void lockHelper();
    void unlockHelper();
};

#endif
