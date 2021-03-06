#include "pmm.hpp"
#include "vmm.hpp"
#include <boot/stivale2.hpp>
#include <memory.hpp>
#include <math.hpp>
#include <strings.hpp>
#include <bitmap.hpp>
#include <lock.hpp>

#include <stdint.h>
#include <stddef.h>

static uint8_t* bitmap; //1 = used, 0 = free
static size_t bitmapSize = 0;
static size_t lastUsablePage = 0;

Lock::lock_t pmm_lock = 0;

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

            if(mmap[i].type != STIVALE2_MMAP_USABLE 
                && mmap[i].type != STIVALE2_MMAP_BOOTLOADER_RECLAIMABLE
                || mmap[i].length < bitmapSize) 
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
            if (mmap[i].type != STIVALE2_MMAP_USABLE 
                && mmap[i].type != STIVALE2_MMAP_BOOTLOADER_RECLAIMABLE)
                continue;

            size_t pageNumber = mmap[i].base / PAGE_SIZE;

            /*  this sets the bit corresponding to the frame number as 0 
                (marking the frame as free)
            */
        
            size_t length = mmap[i].length / PAGE_SIZE;

            for (size_t i = pageNumber; i <= pageNumber + length; i++) {
                toys::clear_bit(bitmap, i);
            }
        }
    }

    void* alloc(size_t count) {
        size_t currentCount = 0;

        Lock::acquire(&pmm_lock);

        for (size_t i = 0; i < bitmapSize*8; i++) {
            if (!toys::is_bit_set(bitmap, i)) {
                currentCount++;

                if (currentCount == count) {
                    size_t page = i - count + 1;
                    
                    for (size_t c = page; c < page + count; c++) {
                        toys::set_bit(bitmap, c);
                    }

                    uint64_t addr = page * PAGE_SIZE;
                    memset((void*)addr + PHYSICAL_BASE_ADDRESS, 0, count * PAGE_SIZE);

                    Lock::release(&pmm_lock);

                    return (void*)addr;
                } 
            } else {
                currentCount = 0;
            }
        }

        Lock::release(&pmm_lock);
    
        return nullptr;
    }

    void free(void* ptr, size_t count) {
        size_t page = (size_t)ptr/PAGE_SIZE;

        Lock::acquire(&pmm_lock);

        for (size_t i = page; i < page + count; i++) {
            toys::clear_bit(bitmap, i);
        }

        Lock::release(&pmm_lock);
    }

    uint64_t get_available_memory() {
        uint64_t free_pages = 0;

        for (size_t i = 0; i < bitmapSize*8; i++) {
            if (!toys::is_bit_set(bitmap, i)) {
                free_pages++;
            }
        }

        return free_pages * PAGE_SIZE;
    }
}
