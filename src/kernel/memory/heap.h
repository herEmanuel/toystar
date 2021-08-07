#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>

#define INITIAL_PAGES 256 // 1MB
#define MAX_POWER 20
#define MIN_POWER 4
#define SMALLEST_SIZE 16

/* buddy allocator for the kernel heap */

struct Block {
    bool free;
    Block* next;
};

class BuddyAllocator {
public:
    Block** bucketList;

    constexpr size_t indexFromOrder(size_t order);
    constexpr size_t sizeFromOrder(size_t order);
    Block* mergeBlocks(Block* block1, Block* block2);
    void splitBlock(size_t order);

public:
    void init();
    void* kmalloc(size_t bytes);
    void* kfree(void* ptr);
    void* krealloc(void* ptr, size_t bytes);

};

#endif
