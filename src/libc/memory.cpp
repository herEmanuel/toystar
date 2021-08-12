#include "memory.hpp"
#include <stddef.h>

void memset(void* ptr, size_t value, size_t bytes) {
    for (size_t i = 0; i < bytes; i++) {
        ((char*)ptr)[i] = value;
    }
}

void memcpy(void* dest, void* src, size_t bytes) {
    for (size_t i = 0; i < bytes; i++) {
        ((char*)dest)[i] = ((char*)src)[i];
    }
}