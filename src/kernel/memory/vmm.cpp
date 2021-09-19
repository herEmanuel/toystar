#include "vmm.hpp"
#include "pmm.hpp"
#include <video.hpp>
#include <strings.hpp>
#include <x86_64/idt.hpp>
#include <memory/heap.hpp>

#include <stddef.h>
#include <stdint.h>

extern "C" void _isr_page_fault();

namespace VMM {

    vmm* kernel_vmm = nullptr; 

    void init() {
        kernel_vmm = new vmm();

        uint64_t pml4;

        asm volatile (
            "mov %%cr3, %0"
            : "=r"(pml4)
        );
     
        kernel_vmm->set_pml4((pml4 + PHYSICAL_BASE_ADDRESS));

        register_interrupt_handler(0xE, (uint64_t)&_isr_page_fault, 0x8E, 0);
    }

    vmm::vmm() {
    }

    vmm::vmm(bool bruh) {
        if (bruh) {
            m_pml4 = (uint64_t*) (PMM::alloc(1) + PHYSICAL_BASE_ADDRESS);
        }
    }

    uint64_t* vmm::get_next_level(uint64_t* currLevelPtr, uint16_t entry) {
        if (!currLevelPtr[entry] & 1) {
            uint64_t allocated = (uint64_t)PMM::alloc(1);
            currLevelPtr[entry] = allocated | 0b111;

            return (uint64_t*)allocated; 
        }
    
        return (uint64_t*)(currLevelPtr[entry] & 0xfffffffffffff000);
    }

    void vmm::map_page(uint64_t virt, uint64_t phys, uint16_t flags) {
        uint16_t pml4e, pdpe, pde, pte;

        pml4e = ((virt >> 39) & 0x1FF);
        pdpe = ((virt >> 30) & 0x1FF);
        pde = ((virt >> 21) & 0x1FF);
        pte = ((virt >> 12) & 0x1FF);

        uint64_t* pdp = get_next_level(m_pml4, pml4e);
        uint64_t* pd = get_next_level(pdp, pdpe);
        uint64_t* pt = get_next_level(pd, pde);

        pt[pte] = phys | flags;
    }

    void vmm::map_range_raw(uint64_t virt, uint64_t phys, size_t length, size_t prot) {
        for (size_t i = 0; i < length; i += PAGE_SIZE) {
            map_page(virt+i, phys+i, prot);
        }
    }

    void vmm::unmap_page(uint64_t virt) {
        map_page(virt, 0, 0);
        invlpg(virt);
    }

    void vmm::unmap_range_raw(uint64_t virt, size_t length) {
        for (size_t i = 0; i < length; i += PAGE_SIZE) {
            unmap_page(virt+i);
        }
    }

    uint64_t vmm::virtual_to_physical(uint64_t virt) {
        uint16_t pml4e, pdpe, pde, pte;
        
        pml4e = ((virt >> 39) & 0x1FF);
        pdpe = ((virt >> 30) & 0x1FF);
        pde = ((virt >> 21) & 0x1FF);
        pte = ((virt >> 12) & 0x1FF);

        uint64_t* pdp = get_next_level(m_pml4, pml4e);
        uint64_t* pd = get_next_level(pdp, pdpe);
        uint64_t* pt = get_next_level(pd, pde);
        
        uint64_t phys = (pt[pte] & 0xfffffffffffff000);
        
        return phys;
    }

    void vmm::map_range(uint64_t virt, uint64_t phys, size_t length, size_t prot, size_t flags) {
        MemArea* range = new MemArea;
        //TODO: anonymus flag and blablabla

        range->base = virt;
        range->limit = length;
        range->prot = prot;
        range->flags = flags;
        ranges.push_back(range);
        
        map_range_raw(virt, phys, length, prot);
    }

    void vmm::switch_pagemap() {
        uint64_t pml4 = (uint64_t)((void*)m_pml4 - PHYSICAL_BASE_ADDRESS);

        asm volatile (
            "mov %0, %%cr3"
            :
            : "r"(pml4)
        );
    }

    void vmm::set_pml4(uint64_t pml4) {
        m_pml4 = (uint64_t*) pml4;
    }

}

extern "C" void isr_page_fault(interrupt_frame* iframe) {
    kprint("Page fault!");

    kprint("Error code: ");
    kprint(iframe->error_code);

    while(true)
        asm("hlt");
}
