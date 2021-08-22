#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stddef.h>
#include <boot/stivale2.hpp>

struct TSS {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved2;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved3;
    uint32_t iobm; //TODO: still don't know exactly what this is
} __attribute__((packed));

struct cpuid_return {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
};

namespace Cpu {
    
    cpuid_return cpuid(uint32_t eax, uint32_t ecx);
    void halt();
    void init_features();
    void bootstrap_cores(stivale2_struct_tag_smp* smpInfo);
    void core_init();
    
}

#endif