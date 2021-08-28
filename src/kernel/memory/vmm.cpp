#include "vmm.hpp"
#include "pmm.hpp"
#include <video.hpp>
#include <strings.hpp>
#include <x86_64/idt.hpp>
#include <memory/heap.hpp>

#include <stddef.h>
#include <stdint.h>

namespace VMM {

    vmm* kernel_vmm = nullptr; 

    //TODO: make my own pml4 
    void init() {
        kernel_vmm = (vmm*) (PMM::alloc(1) + PHYSICAL_BASE_ADDRESS);

        new (kernel_vmm) vmm();

        uint64_t pml4;

        asm volatile (
            "mov %%cr3, %0"
            : "=r"(pml4)
        );
     
        kernel_vmm->setPml4((pml4 + PHYSICAL_BASE_ADDRESS));

        registerInterruptHandler(0xE, (uint64_t)&page_fault_handler, 0x8E, 0);
    }

    vmm::vmm() {
    }

    vmm::vmm(bool bruh) {
        if (bruh) {
            m_pml4 = (uint64_t*) PMM::alloc(1);
            kprint("size: %d\n", ranges.size());
        }
    }

    uint64_t* vmm::getNextLevel(uint64_t* currLevelPtr, uint16_t entry) {
        // kprint("yep next level\n");
        //not present
        if (!currLevelPtr[entry] & 1) {
            //TODO: use heap?
            uint64_t allocated = (uint64_t)PMM::alloc(1);
            currLevelPtr[entry] = allocated | 0b111;

            return (uint64_t*)allocated; 
        }
    
        return (uint64_t*)(currLevelPtr[entry] & 0xfffffffffffff000);
    }

    void vmm::mapPage(uint64_t virt, uint64_t phys, uint16_t flags) {
        uint16_t pml4e, pdpe, pde, pte;

        pml4e = ((virt >> 39) & 0x1FF);
        pdpe = ((virt >> 30) & 0x1FF);
        pde = ((virt >> 21) & 0x1FF);
        pte = ((virt >> 12) & 0x1FF);

        uint64_t* pdp = getNextLevel(m_pml4, pml4e);
        uint64_t* pd = getNextLevel(pdp, pdpe);
        uint64_t* pt = getNextLevel(pd, pde);

        pt[pte] = phys | flags;
    }

    void vmm::mapRangeRaw(uint64_t virt, uint64_t phys, size_t length, size_t prot) {
        //TODO: test
        for (size_t i = 0; i < length; i +=  PAGE_SIZE) {
            mapPage(virt+i, phys+i, prot);
        }
    }

    void vmm::unmapPage(uint64_t virt) {
        mapPage(virt, 0, 0);
        invlpg(virt);
    }

    void vmm::unmapRangeRaw(uint64_t virt, size_t length) {
        for (size_t i = 0; i < length; i += PAGE_SIZE) {
            unmapPage(virt+i);
        }
    }

    uint64_t vmm::virtualToPhysical(uint64_t virt) {
        uint16_t pml4e, pdpe, pde, pte;
        
        pml4e = ((virt >> 39) & 0x1FF);
        pdpe = ((virt >> 30) & 0x1FF);
        pde = ((virt >> 21) & 0x1FF);
        pte = ((virt >> 12) & 0x1FF);

        uint64_t* pdp = getNextLevel(m_pml4, pml4e);
        uint64_t* pd = getNextLevel(pdp, pdpe);
        uint64_t* pt = getNextLevel(pd, pde);
        
        uint64_t phys = (pt[pte] & 0xfffffffffffff000);
        
        return phys;
    }

    void vmm::mapRange(uint64_t virt, uint64_t phys, size_t length, size_t prot, size_t flags) {
        MemArea* range = new MemArea;
        kprint("NOT AGAIN\n");
        //TODO: anonymus flag and blablabla

        range->base = virt;
        range->limit = length;
        range->prot = prot;
        range->flags = flags;
        kprint("ok1\n");
        ranges.push_back(range);
        kprint("ok2\n");
        mapRangeRaw(virt, phys, length, prot);
        kprint("FUCK\n");
    }

    void vmm::switchPagemap() {
        asm volatile (
            "mov %0, %%cr3"
            :
            : "r"(m_pml4)
        );
    }

    void vmm::setPml4(uint64_t pml4) {
        m_pml4 = (uint64_t*) pml4;
    }

}

__INTERRUPT__ void page_fault_handler(interrupt_frame* frame, uint64_t errCode) {
    kprint("Page fault!");

    kprint("Error code: ");
    kprint(errCode);

    while(true)
        asm("hlt");
}
