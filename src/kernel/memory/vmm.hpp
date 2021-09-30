#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <stddef.h>
#include <x86_64/idt.hpp>
#include <vector.hpp>

#define PHYSICAL_BASE_ADDRESS 0xffff800000000000
#define KERNEL_BASE 0xffffffff80000000
#define PAGE_SIZE 4096

namespace VMM {

    struct MemArea {
        uint64_t base;
        uint64_t limit;
        size_t prot;
        size_t flags;
    };

    void init();

    class vmm {
    public:
        uint64_t* m_pml4;
        toys::vector<MemArea*> m_ranges;

        /* used to store the addresses of the frames where we store the page tables, so that we 
           can easily free them memory when destroying the pagemap
         */
        toys::vector<uint64_t*> page_tables_addr;

        uint64_t* get_next_level(uint64_t* currLevelPtr, uint16_t entry);

        inline uint16_t get_pml4e(uint64_t virtual_addr) {
            return ((virtual_addr >> 39) & 0x1FF);
        }

        inline uint16_t get_pdpe(uint64_t virtual_addr) {
            return ((virtual_addr >> 30) & 0x1FF);
        }

        inline uint16_t get_pde(uint64_t virtual_addr) {
            return ((virtual_addr >> 21) & 0x1FF);
        }

        inline uint16_t get_pte(uint64_t virtual_addr) {
            return ((virtual_addr >> 12) & 0x1FF);
        }

    public:
        vmm();

        vmm(bool non_default);

        ~vmm();

        void map_page(uint64_t virt, uint64_t phys, uint16_t flags);

        void map_range_raw(uint64_t virt, uint64_t phys, size_t length, size_t prot);

        void unmap_page(uint64_t virt);

        void unmap_range_raw(uint64_t virt, size_t length);

        void map_range(uint64_t virt, uint64_t phys, size_t length, size_t prot, size_t flags);
        
        uint64_t virtual_to_physical(uint64_t virt);

        vmm* duplicate();

        void destroy();

        void switch_pagemap();

        void set_pml4(uint64_t pml4);
        
        toys::vector<MemArea*> get_ranges();

        inline void invlpg(uint64_t addr) {
            asm volatile ("invlpg (%0)" :: "r"(addr));
        }
    };

    extern vmm* kernel_vmm; 
}

extern "C" void isr_page_fault(interrupt_frame* iframe);

#endif