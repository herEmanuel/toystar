#include "heap.hpp"
#include "pmm.hpp"
#include "vmm.hpp"
#include <memory.hpp>
#include <strings.hpp>
#include <video.hpp>
#include <utils.hpp>

#include <stdint.h>
#include <stddef.h>

//TODO: do more testing

Heap::BuddyAllocator* allocator = nullptr;

namespace Heap {

    void init() {
        allocator = (BuddyAllocator*) PMM::alloc(1);

        if (!allocator) {
            Toystar::utils::panic("could not initialize the kernel heap");
        }

        new (allocator) BuddyAllocator();
    }

    BuddyAllocator::BuddyAllocator() {
        bucketList = (Block**) (PMM::alloc(1) + PHYSICAL_BASE_ADDRESS);

        Block* firstBlock = (Block*) (PMM::alloc(INITIAL_PAGES) + PHYSICAL_BASE_ADDRESS);

        bucketList[0] = firstBlock;
        bucketList[0]->size = 1 << MAX_POWER;
        bucketList[0]->next = nullptr;

        baseAddr = (uint64_t) firstBlock;
    }

    size_t BuddyAllocator::indexFromOrder(size_t order) {
        return MAX_POWER - order;
    }

    int BuddyAllocator::findBuddy(size_t order, size_t i) {
        if (order <= MIN_POWER) {
            return -1;
        }

        size_t index = indexFromOrder(order);

        if (!bucketList[index]) {
            return -1;
        }

        Block* curr = bucketList[index];
        
        while (i--) {
            curr = curr->next;
        }

        if (curr == nullptr) {
            return -1;
        }

        uint64_t buddy = ((uint64_t) curr) ^ (1 << order);

        curr = bucketList[index];
        i = 0;

        while (curr != nullptr) {
            if ((uint64_t) curr == buddy) {
                return i;
            }

            curr = curr->next;
            i++;
        }

        return -1;
    }

    bool BuddyAllocator::splitBlock(size_t order) {
        if (order <= MIN_POWER && order > MAX_POWER) {
            return false;
        }

        size_t index = indexFromOrder(order);

        if (!bucketList[index]) {
            return false;
        }

        Block* block = bucketList[index];
        bucketList[index] = block->next;

        uint8_t* tmp = (uint8_t*) block;
        Block* block2 = (Block*) &tmp[(1 << order)/2];
        block2->next = bucketList[index + 1];
        block2->size = (1 << order)/2;

        block->next = block2;
        block->size = (1 << order)/2;
        bucketList[index + 1] = block;
    
        return true;
    }

    bool BuddyAllocator::tryToMerge(size_t order) {
        // kprint("MERGING ORDER: %d\n", order);

        if (order > MAX_POWER) {
            return false;
        }

        int buddy = 0;
        bool merged = false;
        size_t index = indexFromOrder(order);

        size_t i = 0;
        Block* curr = bucketList[index];
    
        while (curr != nullptr) {
            buddy = findBuddy(order, i);
        
            if (buddy >= 0) {
                Block** p = &bucketList[index];
                Block* buddyBlock = bucketList[index];
            
                while (buddy--) {
                    //Remove curr block
                    if (*p == curr) {
                        *p = buddyBlock->next;
                        buddyBlock = buddyBlock->next;
                        continue;
                    }

                    buddyBlock = buddyBlock->next;
                    p = &buddyBlock->next;
                }
                
                *p = buddyBlock->next;

                Block* block = curr;

                curr = curr->next;
                if (curr == buddyBlock) {
                    curr = curr->next;
                }
                i++;

                if ((uint64_t)buddyBlock > (uint64_t)block) {
                    block->size = (1 << (order + 1));
                    block->next = bucketList[index-1];
                    bucketList[index-1] = block;
                } else {
                    buddyBlock->size = (1 << (order + 1));
                    buddyBlock->next = bucketList[index-1];
                    bucketList[index-1] = buddyBlock;
                }

                merged = true;
            } else {
                curr = curr->next;
                i++;
            }
        }

        return merged;
    }

    void* BuddyAllocator::kmalloc(size_t bytes) {
        size_t order = 4;
        size_t size = bytes + sizeof(Block); 

        while ((1 << order) < size) {
            order++;
        }

        size_t index = indexFromOrder(order);

        if (!bucketList[index]) {
            size_t t = order + 1;

            while (!splitBlock(t++)) {
                if (t > MAX_POWER) { return nullptr; }
            }

            t--;
            while (--t > order) {
                splitBlock(t);
            }
        } 

        Block* allocated = bucketList[index];
        bucketList[index] = allocated->next;

        allocated->next = nullptr;

        return (void*)allocated + sizeof(Block);
    }

    void BuddyAllocator::kfree(void* ptr) {
        if ((uint64_t) ptr < baseAddr || 
            (uint64_t) ptr > baseAddr + (1 << MAX_POWER)) 
            return;
        
        Block* block = (Block*) (ptr - sizeof(Block));
        size_t order = 4;
        
        while ((1 << order++) < block->size);
        order--;

        size_t index = indexFromOrder(order);
    
        block->next = bucketList[index];
        bucketList[index] = block;

        while (order < MAX_POWER && tryToMerge(order++));
    }

    void* BuddyAllocator::krealloc(void* ptr, size_t bytes) {
        if ((uint64_t) ptr < baseAddr || 
            (uint64_t) ptr > baseAddr + (1 << MAX_POWER)) 
            return nullptr;
        
        Block* block = (Block*) (ptr - sizeof(Block));

        void* newPtr = kmalloc(bytes);

        if (bytes > block->size) {
            memcpy(newPtr, ptr, block->size);
        } else {
            memcpy(newPtr, ptr, bytes);
        }

        kfree(ptr);
        return newPtr;
    }
}

void* krealloc(void* ptr, size_t bytes) {
    return allocator->krealloc(ptr, bytes);
}

void* operator new(size_t size) { return allocator->kmalloc(size); }

void* operator new[](size_t size) { return allocator->kmalloc(size); }

void* operator new(size_t size, void* p) { return p; }

void* operator new[](size_t size, void* p) { return p; }

void operator delete(void* ptr) { allocator->kfree(ptr); }

void operator delete(void* ptr, size_t) { allocator->kfree(ptr); }

void operator delete[](void* ptr) { allocator->kfree(ptr); }

void operator delete[](void* ptr, size_t) { allocator->kfree(ptr); }