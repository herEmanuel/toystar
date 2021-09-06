#ifndef PMM_H
#define PMM_H

#include <boot/stivale2.hpp>
#include <stdint.h>
#include <stddef.h>

namespace PMM {

    void init(stivale2_mmap_entry* mmap, uint64_t entries);
    void* alloc(size_t count);
    void free(void* ptr, size_t count);
    uint64_t get_available_memory();
}

#endif