#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <stddef.h>
#include <x86_64/idt.hpp>

#define PAGE_SIZE 4096
#define PHYSICAL_BASE_ADDRESS 0xffff800000000000

struct Pagemap {
    uint64_t* pml4;
};

class VMM {
    struct Pagemap* currentPagemap;

    uint64_t* getNextLevel(uint64_t* currLevelPtr, uint16_t entry);
    
public:
    void init();
    void mapPage(uint64_t virt, uint64_t phys, uint16_t flags);
    void mapRange(uint64_t virt, uint64_t phys, size_t count);
    void unmapPage(uint64_t virt);
    void unmapRange(uint64_t virt, size_t count);
    uint64_t virtualToPhysical(uint64_t virt);
    Pagemap* newPagemap();
    void switchPagemap(struct Pagemap* pagemap);
    void invlpg(uint64_t addr);
};

__INTERRUPT__ void page_fault_handler(interrupt_frame* frame, uint64_t errCode);

#endif