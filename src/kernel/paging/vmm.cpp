#include "vmm.h"
#include "pmm.h"
#include <stddef.h>
#include <stdint.h>

void VMM::init() {
    uint64_t pml4;

    asm volatile (
        "mov %%cr3, %0"
        : "=r"(pml4)
    );

    currentPagemap->pml4 = (uint64_t*) pml4 + PHYSICAL_BASE_ADDRESS;
}

uint64_t* VMM::getNextLevel(uint64_t* currLevelPtr, uint16_t entry) {
    //not present
    if (!currLevelPtr[entry] & 1) {
        //TODO: use heap?
        uint64_t allocated = (uint64_t)PMM::alloc(1);
        currLevelPtr[entry] = allocated | 0b111;

        return (uint64_t*)allocated; 
    }

    return (uint64_t*)currLevelPtr[entry] & 0xfffffffffffff000;
}


void VMM::mapPage(uint64_t virt, uint64_t phys, uint16_t flags) {
    uint16_t pml4e, pdpe, pde, pte;

    pml4e = ((virt >> 39) & 0x1FF);
    pdpe = ((virt >> 31) & 0x1FF);
    pde = ((virt >> 23) & 0x1FF);
    pte = ((virt >> 15) & 0x1FF);

    uint64_t* pdp = getNextLevel(currentPagemap->pml4, pml4e);
    uint64_t* pd = getNextLevel(pdp, pdpe);
    uint64_t* pt = getNextLevel(pd, pde);

    pt[pte] = phys | flags;
}

void VMM::mapRange(uint64_t virt, uint64_t phys, size_t count) {
    //TODO: test
    for (size_t i = 0; i < count * PAGE_SIZE; i +=  PAGE_SIZE) {
        mapPage(virt+i, phys+i);
    }
}

void VMM::unmapPage(uint64_t virt) {
    mapPage(virt, 0, 0);
}

void VMM::unmapRange(uint64_t virt, size_t count) {
    for (size_t i = 0; i < count * PAGE_SIZE; i += PAGE_SIZE) {
        unmapPage(virt+i);
    }
}

Pagemap* VMM::newPagemap() {
    struct Pagemap newPagemap;
    //TODO: use heap?
    newPagemap.pml4 = (uint64_t*)PMM::alloc(1);
    return newPagemap;
}

void VMM::switchPagemap(struct Pagemap pagemap) {
    currentPagemap = pagemap;

    asm volatile (
        "mov %0, %%cr3"
        :
        : "r"(pagemap.pml4)
    );
}
