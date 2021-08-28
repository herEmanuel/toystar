#include "cpu.hpp"

#include <stdint.h>
#include <stddef.h>
#include <video.hpp>
#include <boot/stivale2.hpp>
#include <drivers/hpet.hpp>
#include <memory/pmm.hpp>
#include <memory/heap.hpp>
#include <lock.hpp>
#include <x86_64/gdt.hpp>
#include <x86_64/idt.hpp>

namespace Cpu {

    cpuid_return cpuid(uint32_t eax, uint32_t ecx) {
        cpuid_return ret = {0, 0, 0, 0};

        asm volatile(
            "cpuid" 
            : "=a"(ret.rax), "=b"(ret.rbx), "=c"(ret.rcx), "=d"(ret.rdx) 
            : "a"(eax), "c"(ecx)
        );

        return ret;
    }

    void init_features() {
        cpuid_return sse = cpuid(1, 0);

        if (!(sse.rdx & (1 << 25))) {
            return;
        }

        //this enables SSE
        uint64_t cr0 = 0;
        asm volatile("movq %%cr0, %0" : "=r"(cr0));
        cr0 &= ~(1 << 2);
        cr0 |= 2;
        asm volatile("movq %0, %%cr0" :: "r"(cr0));

        uint64_t cr4 = 0;
        asm volatile("movq %%cr4, %0" : "=r"(cr4));
        cr4 |= (1 << 9);
        cr4 |= (1 << 10);
        asm volatile("movq %0, %%cr4" :: "r"(cr4));
    }

    void halt() {
        while(true)
            asm("hlt");
    }

    size_t cpus = 0;

    //TODO: finish it
    void bootstrap_cores(stivale2_struct_tag_smp* smpInfo) {
        cpus++;
        for (size_t i = 0; i < smpInfo->cpu_count; i++) {
            void* stack = PMM::alloc(1);

            cpu* cpu_data = new cpu;
            cpu_data->lapic_id = smpInfo->smp_info[i].lapic_id;
            cpu_data->pid = -1;
            cpu_data->tid = -1;

            smpInfo->smp_info[i].target_stack = reinterpret_cast<uint64_t>(stack);
            smpInfo->smp_info[i].extra_argument = (uint64_t) cpu_data;
            __atomic_store(&smpInfo->smp_info[i].goto_address, (uint64_t*)&core_init, __ATOMIC_RELAXED);
        }

        while(cpus != 4);
        kprint("cpus: %d\n", cpus);
    }

    void core_init() {
        load_gdt();
        load_idt();

        init_features();

        __atomic_fetch_add(&cpus, 1, __ATOMIC_RELAXED);

        halt();
    }

}