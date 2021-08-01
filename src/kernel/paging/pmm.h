#ifndef PMM_H
#define PMM_H

#include <boot/stivale2.h>
#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096
#define PHYSICAL_BASE_ADDRESS 0xffff800000000000

namespace PMM {
    void init(stivale2_mmap_entry* mmap, uint64_t entries);
    void* alloc(size_t count);
    void free(void* ptr, size_t count);
}

#endif