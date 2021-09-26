#include "memory.hpp"

#include <stddef.h>
#include <stdint.h>

void memset(void* ptr, size_t value, size_t bytes) {
    for (size_t i = 0; i < bytes; i++) {
        ((uint8_t*)ptr)[i] = value;
    }
}

void memcpy(void* dest, void* src, size_t bytes) {
    for (size_t i = 0; i < bytes; i++) {
        ((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
    }
}

void memcpy8(void* dest, void* src, size_t bytes) {
    for (size_t i = 0; i < bytes/8; i++) {
        ((uint64_t*)dest)[i] = ((uint64_t*)src)[i];
    }
}