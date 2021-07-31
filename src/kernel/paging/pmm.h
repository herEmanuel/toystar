#ifndef PMM_H
#define PMM_H

#include <boot/stivale2.h>
#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096
#define PHYSICAL_BASE_ADDRESS 0xffff800000000000

void pmmInit(stivale2_mmap_entry* mmap, uint64_t entries);
void* pmmAlloc(size_t count);
void pmmFree(void* ptr, size_t count);

#endif