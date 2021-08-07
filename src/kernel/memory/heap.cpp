#include "heap.h"
#include "pmm.h"
#include <libc/memory.h>
#include <kernel/vga.h>
#include <stdint.h>
#include <stddef.h>

constexpr size_t BuddyAllocator::indexFromOrder(size_t order) {
    return MAX_POWER - order;
}

constexpr size_t BuddyAllocator::sizeFromOrder(size_t order) {
    return 1 << order;
}

void BuddyAllocator::init() {
    bucketList = (Block**) PMM::alloc(1);

    Block* firstBlock = (Block*) PMM::alloc(INITIAL_PAGES);

    bucketList[0] = firstBlock;
    bucketList[0]->free = true;
    bucketList[0]->next = nullptr;
    kprint("addr: ");
    kprint((size_t)bucketList[0]);
}

void BuddyAllocator::splitBlock(size_t order) {
    
    if (order <= MIN_POWER) {
        return;
    }

    size_t index = indexFromOrder(order);

    if (!bucketList[index]) {
        return;
    }

    Block* block = bucketList[index];
    bucketList[index] = block->next;

    uint8_t* tmp = (uint8_t*) block;
    Block* block2 = (Block*) &tmp[sizeFromOrder(order)/2];
    block2->free = true;
    block2->next = bucketList[index + 1];

    block->next = block2;
    bucketList[index + 1] = block;
    // kprint("\n");
    // kprint("block1: ");
    // kprint((size_t)block);
    // kprint("block2: ");
    // kprint((size_t)block2);
    // kprint("\n");
}

// Block* BuddyAllocator::mergeBlocks(Block* block1, Block* block2) {
//     block1->size *= 2;
//     block1->next = block2->next;
    
//     for (size_t i = 0; i < MAX_BLOCKS; i++) {
//         if (blockList[i] == block2) {
//             blockList[i] = blockList[blocksCount-1];
//             blockList[blocksCount - 1] = nullptr;
//         }
//     }

//     return block1;
// }

// void* BuddyAllocator::kmalloc(size_t bytes) {
//     Block* currBlock = blockList[0];

//     while (currBlock != nullptr) {
        
//     }   

//     return nullptr;
// }
