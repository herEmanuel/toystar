#include "cpu.hpp"

#include <stdint.h>
#include <stddef.h>
#include <video.hpp>
#include <boot/stivale2.hpp>
#include <drivers/hpet.hpp>
#include <memory/pmm.hpp>
#include <memory/heap.hpp>
#include <x86_64/gdt.hpp>
#include <x86_64/idt.hpp>
#include <x86_64/apic.hpp>

namespace Cpu {

    size_t cpus = 1;

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

    void hang() {
        while(true)
            asm("hlt");
    }

    void bootstrap_cores(stivale2_struct_tag_smp* smpInfo) {
        for (size_t i = 0; i < smpInfo->cpu_count; i++) {

            cpu* cpu_data = new cpu;
            cpu_data->lapic_id = smpInfo->smp_info[i].lapic_id;
            cpu_data->running_thread = nullptr;
            cpu_data->running_process = nullptr;

            if (smpInfo->smp_info[i].lapic_id == smpInfo->bsp_lapic_id) {
                TSS* tss = new TSS;
                tss->rsp0 = (uint64_t) (PMM::alloc(4) + PHYSICAL_BASE_ADDRESS);

                load_tss((uint64_t)tss);

                set_gs((uint64_t)cpu_data);
                continue;
            }

            void* stack = PMM::alloc(4) + PHYSICAL_BASE_ADDRESS;

            smpInfo->smp_info[i].target_stack = reinterpret_cast<uint64_t>(stack);
            smpInfo->smp_info[i].extra_argument = reinterpret_cast<uint64_t>(cpu_data);

            __atomic_store_n(&smpInfo->smp_info[i].goto_address, &core_init, __ATOMIC_RELAXED);
        }

        while(cpus != smpInfo->cpu_count);
        print("All cores have been initialized\n");
    }

    void core_init(stivale2_smp_info* smpInfo) {     
        load_gdt();
        load_idt();

        init_features();

        set_gs(smpInfo->extra_argument);

        Apic::localApic->enable();

        __atomic_fetch_add(&cpus, 1, __ATOMIC_RELAXED);

        hang();
    }

}