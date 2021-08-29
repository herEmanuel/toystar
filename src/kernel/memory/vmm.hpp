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
        uint64_t* m_pml4;
        toys::vector<MemArea*> ranges;

        uint64_t* getNextLevel(uint64_t* currLevelPtr, uint16_t entry);
    public:
        vmm();
        vmm(bool bruh);
        void mapPage(uint64_t virt, uint64_t phys, uint16_t flags);
        void mapRangeRaw(uint64_t virt, uint64_t phys, size_t length, size_t prot);
        void unmapPage(uint64_t virt);
        void unmapRangeRaw(uint64_t virt, size_t length);
        void mapRange(uint64_t virt, uint64_t phys, size_t length, size_t prot, size_t flags);
        uint64_t virtualToPhysical(uint64_t virt);
        void switchPagemap();

        void setPml4(uint64_t pml4);

        inline void invlpg(uint64_t addr) {
            asm volatile ("invlpg (%0)" :: "r"(addr));
        }
    };

    extern vmm* kernel_vmm; 
}

__INTERRUPT__ void page_fault_handler(interrupt_frame* frame, uint64_t errCode);

#endif