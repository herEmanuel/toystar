#include "pmm.hpp"
#include <boot/stivale2.hpp>
#include <video.hpp>
#include <memory.hpp>
#include <math.hpp>
#include <strings.hpp>

#include <stdint.h>
#include <stddef.h>

static uint8_t* bitmap; //1 = used, 0 = free
static size_t bitmapSize = 0;
static size_t lastUsablePage = 0;

bool isBitUnset(size_t bit);

namespace PMM {
    
    void init(stivale2_mmap_entry* mmap, size_t entries) {

        for (size_t i = 0; i < entries; i++) {
            if (mmap[i].type != STIVALE2_MMAP_USABLE 
            && mmap[i].type != STIVALE2_MMAP_KERNEL_AND_MODULES) 
                continue;

            uint64_t top = mmap[i].base + mmap[i].length;

            if (top > lastUsablePage) 
                lastUsablePage = top;
        }

        bitmapSize = DIV_CEIL(lastUsablePage/PAGE_SIZE, 8);
        
        for (size_t i = 0; i < entries; i++) {
            if(mmap[i].type != STIVALE2_MMAP_USABLE || mmap[i].length < bitmapSize) 
                continue; 
            
            bitmap = (uint8_t*)mmap[i].base + PHYSICAL_BASE_ADDRESS;
            size_t bitmapPages = DIV_CEIL(bitmapSize, PAGE_SIZE);

            //mark everything as not usable
            memset(bitmap, 0xFF, bitmapSize);
            
            mmap[i].base += bitmapPages * PAGE_SIZE;
            mmap[i].length -= bitmapPages * PAGE_SIZE;
            break;
        }
    
        for (size_t i = 0; i < entries; i++) {
            if (mmap[i].type != STIVALE2_MMAP_USABLE)
                continue;

            size_t pageNumber = mmap[i].base / PAGE_SIZE;

            /*  this sets the bit corresponding to the frame number as 0 
                (marking the frame as free)
            */
        
            size_t length = mmap[i].length / PAGE_SIZE;

            //TODO: this can be optimized (a lot)
            for (size_t i = pageNumber; i <= pageNumber + length; i++) {
                //flawed
                if ((i%8) != 0) {
                    bitmap[i/8] ^= (1 << (8 - i%8));
                } else {
                    bitmap[i/8 - 1] ^= 1;
                }
            }
        }
    }

    void* alloc(size_t count) {
        size_t currentCount = 0;

        for (size_t i = 0; i < bitmapSize*8; i++) {
            if (isBitUnset(i)) {
                currentCount++;

                if (currentCount == count) {
                    size_t page = i - count + 1;
                    
                    for (size_t c = page; c < page + count; c++) {
                        if ((c%8) != 0) {
                            bitmap[c/8] |= (1 << (8 - c%8));
                        } else {
                            bitmap[c/8 - 1] |= 1;
                        }
                    }

                    uint64_t addr = page * PAGE_SIZE;
                    memset((void*)addr + PHYSICAL_BASE_ADDRESS, 0, count * PAGE_SIZE);

                    return (void*)addr;
                } 
            } else {
                currentCount = 0;
            }
        }
    
        return nullptr;
    }

    void free(void* ptr, size_t count) {
        size_t page = (size_t)ptr/PAGE_SIZE;

        for (size_t i = page; i < page + count; i++) {
            //flawed
            if ((i%8) != 0) {
                bitmap[i/8] ^= (1 << (8 - i%8));
            } else {
                bitmap[i/8 - 1] ^= 1;
            }
        }
    }
}

bool isBitUnset(size_t bit) {
    size_t i = bit/8;

    if (!bit) {
        if (!(bitmap[0] & 0b10000000)) { return true; }
    }
    else if (bit%8 == 0) {
        if (!(bitmap[i-1] & 1)) { return true; }
    } else {
        if (!(bitmap[i] & (1 << (8 - (bit%8))))) { return true; }
    }

    return false;
}