#ifndef LOCK_H
#define LOCK_H

namespace Lock {

    typedef bool* lock_t;

    inline void acquire(lock_t lock) {
        while (__atomic_test_and_set(lock, __ATOMIC_ACQUIRE));
    }

    inline void release(lock_t lock) {
        __atomic_clear(lock, __ATOMIC_RELEASE);
    }

}

#endif