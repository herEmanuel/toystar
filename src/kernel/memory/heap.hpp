#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>

#define INITIAL_PAGES 256 // 1MB
#define MAX_POWER 20
#define MIN_POWER 4

/* A buddy allocator for the kernel heap */

namespace Heap {

    void init();

    struct Block {
        size_t size;
        Block* next;
        uint16_t reserved;
    };

    class BuddyAllocator {
        Block** bucketList;
        uint64_t baseAddr;

        size_t index_from_order(size_t order);
        int find_buddy(size_t order, size_t i);
        bool try_to_merge(size_t order);
        bool split_block(size_t order);

    public:
        BuddyAllocator();

        void* kmalloc(size_t bytes);
        void kfree(void* ptr);
        void* krealloc(void* ptr, size_t bytes);
    };

}

extern Heap::BuddyAllocator* allocator;

void* krealloc(void* ptr, size_t bytes);

inline void* operator new(size_t size) { return allocator->kmalloc(size); }
inline void* operator new(size_t size, void* p) { return p; }
inline void* operator new[](size_t size) { return allocator->kmalloc(size); }
inline void* operator new[](size_t size, void* p) { return p; }

inline void operator delete(void* ptr) { allocator->kfree(ptr); }
inline void operator delete(void* ptr, size_t) { allocator->kfree(ptr); }
inline void operator delete[](void* ptr) { allocator->kfree(ptr); }
inline void operator delete[](void* ptr, size_t) { allocator->kfree(ptr); }

#endif
