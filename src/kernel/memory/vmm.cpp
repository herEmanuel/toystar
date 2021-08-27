#include "vmm.hpp"
#include "pmm.hpp"
#include <video.hpp>
#include <strings.hpp>
#include <x86_64/idt.hpp>
#include <memory/heap.hpp>

#include <stddef.h>
#include <stdint.h>

namespace VMM {

    VirtualMemoryManager* vmm = nullptr; 

    void init() {
        vmm = new VirtualMemoryManager();
    }

    //TODO: make my own pml4 
    VirtualMemoryManager::VirtualMemoryManager() {
        uint64_t pml4;

        asm volatile (
            "mov %%cr3, %0"
            : "=r"(pml4)
        );

        m_pagemap->pml4 = (uint64_t*) (pml4 + PHYSICAL_BASE_ADDRESS);

        registerInterruptHandler(0xE, (uint64_t)&page_fault_handler, 0x8E, 0);
    }

    uint64_t* VirtualMemoryManager::getNextLevel(uint64_t* currLevelPtr, uint16_t entry) {
        //not present
        if (!currLevelPtr[entry] & 1) {
            //TODO: use heap?
            uint64_t allocated = (uint64_t)PMM::alloc(1);
            currLevelPtr[entry] = allocated | 0b111;

            return (uint64_t*)allocated; 
        }
    
        return (uint64_t*)(currLevelPtr[entry] & 0xfffffffffffff000);
    }

    void VirtualMemoryManager::mapPage(uint64_t virt, uint64_t phys, uint16_t flags) {
        uint16_t pml4e, pdpe, pde, pte;

        pml4e = ((virt >> 39) & 0x1FF);
        pdpe = ((virt >> 30) & 0x1FF);
        pde = ((virt >> 21) & 0x1FF);
        pte = ((virt >> 12) & 0x1FF);

        uint64_t* pdp = getNextLevel(m_pagemap->pml4, pml4e);
        uint64_t* pd = getNextLevel(pdp, pdpe);
        uint64_t* pt = getNextLevel(pd, pde);

        pt[pte] = phys | flags;
    }

    void VirtualMemoryManager::mapRangeRaw(uint64_t virt, uint64_t phys, size_t length, size_t prot) {
        //TODO: test
        for (size_t i = 0; i < length; i +=  PAGE_SIZE) {
            mapPage(virt+i, phys+i, prot);
        }
    }

    void VirtualMemoryManager::unmapPage(uint64_t virt) {
        mapPage(virt, 0, 0);
        invlpg(virt);
    }

    void VirtualMemoryManager::unmapRangeRaw(uint64_t virt, size_t length) {
        for (size_t i = 0; i < length; i += PAGE_SIZE) {
            unmapPage(virt+i);
        }
    }

    uint64_t VirtualMemoryManager::virtualToPhysical(uint64_t virt) {
        uint16_t pml4e, pdpe, pde, pte;
        
        pml4e = ((virt >> 39) & 0x1FF);
        pdpe = ((virt >> 30) & 0x1FF);
        pde = ((virt >> 21) & 0x1FF);
        pte = ((virt >> 12) & 0x1FF);

        uint64_t* pdp = getNextLevel(m_pagemap->pml4, pml4e);
        uint64_t* pd = getNextLevel(pdp, pdpe);
        uint64_t* pt = getNextLevel(pd, pde);
        
        uint64_t phys = (pt[pte] & 0xfffffffffffff000);
        
        return phys;
    }

    void VirtualMemoryManager::mapRange(uint64_t virt, uint64_t phys, size_t length, size_t prot, size_t flags) {
        MemArea* range = new MemArea;

        //TODO: anonymus flag and blablabla

        range->base = virt;
        range->limit = length;
        range->prot = prot;
        range->flags = flags;

        m_pagemap->ranges.push_back(range);

        mapRangeRaw(virt, phys, length, prot);
    }

    Pagemap* VirtualMemoryManager::newPagemap() {
        struct Pagemap* newPagemap;
        //TODO: use heap?
        newPagemap->pml4 = (uint64_t*)PMM::alloc(1);
        return newPagemap;
    }

    void VirtualMemoryManager::switchPagemap(struct Pagemap* pagemap) {
        m_pagemap = pagemap;

        asm volatile (
            "mov %0, %%cr3"
            :
            : "r"(pagemap->pml4)
        );
    }

}

__INTERRUPT__ void page_fault_handler(interrupt_frame* frame, uint64_t errCode) {
    kprint("Page fault!");

    kprint("Error code: ");
    kprint(errCode);

    while(true)
        asm("hlt");
}
