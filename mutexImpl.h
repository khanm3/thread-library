#ifndef _MUTEXIMPL_H
#define _MUTEXIMPL_H

#include "mutex.h"
#include "types.h"

class mutex::impl {
public:
    ThreadStatePtr ownerState;
    std::queue<TcbPtr> lockQueue;

    impl();
    void lockHelper();
    void unlockHelper();
};

#endif
