#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

void memset(void* ptr, size_t value, size_t bytes);
void memcpy(void* dest, void* src, size_t bytes);
void memcpy8(void* dest, void* src, size_t bytes);

#endif