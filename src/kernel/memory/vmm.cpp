#include "vmm.hpp"
#include "pmm.hpp"
#include <utils.hpp>
#include <video.hpp>
#include <x86_64/idt.hpp>
#include <memory/heap.hpp>

#include <stddef.h>
#include <stdint.h>
#include <math.hpp>
#include <memory.hpp>
#include <strings.hpp>

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

    vmm::vmm(bool non_default) {
        m_pml4 = (uint64_t*) (PMM::alloc(1) + PHYSICAL_BASE_ADDRESS);

        //copies the mapping of 0xffff800000000000 from the kernel pagemap
        m_pml4[256] = kernel_vmm->m_pml4[256]; 

        //copies the mapping of 0xffffffff80000000 from the kernel pagemap
        m_pml4[511] = kernel_vmm->m_pml4[511];
    }

    vmm::~vmm() {
        delete m_pml4;
    } 

    uint64_t* vmm::get_next_level(uint64_t* currLevelPtr, uint16_t entry) {
        if (!((uint64_t)currLevelPtr & PHYSICAL_BASE_ADDRESS)) {
            currLevelPtr += PHYSICAL_BASE_ADDRESS / sizeof(uint64_t);
        }

        if (!currLevelPtr[entry] & 1) {
            uint64_t allocated = (uint64_t) PMM::alloc(1);
            currLevelPtr[entry] = allocated | 0b111;

            page_tables_addr.push_back((uint64_t*)allocated);

            return (uint64_t*) (allocated + PHYSICAL_BASE_ADDRESS); 
        }
    
        return (uint64_t*) ((currLevelPtr[entry] & 0xfffffffffffff000) + PHYSICAL_BASE_ADDRESS);
    }

    void vmm::map_page(uint64_t virt, uint64_t phys, uint16_t flags) {
        // log("a");
        uint16_t pml4e, pdpe, pde, pte;

        pml4e = get_pml4e(virt);
        pdpe  = get_pdpe(virt);
        pde   = get_pde(virt);
        pte   = get_pte(virt);
        
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
        
        pml4e = get_pml4e(virt);
        pdpe  = get_pdpe(virt);
        pde   = get_pde(virt);
        pte   = get_pte(virt);

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
        m_ranges.push_back(range);
        
        map_range_raw(virt, phys, length, prot);
    }

    vmm* vmm::duplicate() {
        vmm* new_pagemap = new vmm(true);
        
        for (size_t i = 0; i < m_ranges.size(); i++) {
            MemArea* range = m_ranges[i];

            size_t page_amount = DIV_CEIL(range->limit, PAGE_SIZE);
            uint64_t base = range->base & ~(4095);

            for (size_t t = 0; t < page_amount; t++) {
                void* addr = PMM::alloc(1);
                void* src = (void*)(base + t*PAGE_SIZE);

                memcpy8(addr + PHYSICAL_BASE_ADDRESS, src, PAGE_SIZE);
            
                new_pagemap->map_page((uint64_t)src, (uint64_t)addr, range->prot);
            }

            MemArea* new_range = new MemArea;
            
            memcpy(new_range, range, sizeof(MemArea));
            
            new_pagemap->get_ranges().push_back(new_range);
        }

        return new_pagemap;
    }

    void vmm::destroy() {
        for (size_t i = 0; i < m_ranges.size(); i++) {
            MemArea* range = m_ranges[i];
            size_t page_amount = DIV_CEIL(range->limit, PAGE_SIZE);

            void* physical_addr = (void*) virtual_to_physical(range->base);
            PMM::free(physical_addr, page_amount);

            delete range;
        }

        // for (size_t i = 0; i < page_tables_addr.size(); i++) {
        //     PMM::free(page_tables_addr[i], 1);
        // // }

        this->~vmm();
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

    toys::vector<MemArea*> vmm::get_ranges() {
        return m_ranges;
    }

}

extern "C" void isr_page_fault(interrupt_frame* iframe) {
    print("Page fault!");

    print("Error code: %d\n", iframe->error_code);

    uint64_t cr2;
    asm volatile("movq %%cr2, %0" : "=r"(cr2));

    print("CR2: %x\n", cr2);

    while(true)
        asm("hlt");
}
