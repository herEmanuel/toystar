#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>

#define INITIAL_PAGES 256 // 1MB
#define MAX_POWER 20
#define MIN_POWER 4

/* A buddy allocator for the kernel heap */

struct Block {
    size_t size;
    Block* next;
};

class BuddyAllocator {
public:
    Block** bucketList;
    uint64_t baseAddr;

    constexpr size_t indexFromOrder(size_t order);
    int findBuddy(size_t order, size_t i);
    bool tryToMerge(size_t order);
    bool splitBlock(size_t order);

public:
    void init();
    void* kmalloc(size_t bytes);
    void kfree(void* ptr);
    void* krealloc(void* ptr, size_t bytes);
};

#endif
