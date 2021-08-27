#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stddef.h>
#include <boot/stivale2.hpp>
#include <memory/vmm.hpp>

#define GSBase 0xC0000101

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

struct context {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

struct cpu {
    uint8_t lapic_id;
    size_t tid;
    size_t pid;
    uint64_t kernel_stack;
    uint64_t user_stack;
    VMM::Pagemap* pagemap;
};

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
    
    inline uint64_t rdmsr(uint32_t msr) {
        uint32_t edx, eax;

        asm volatile("rdmsr" : "=d"(edx), "=a"(eax) : "c"(msr));

        return eax | (edx << 32);
    }

    inline void wrmsr(uint32_t msr, uint64_t value) {
        uint32_t eax = value;
        uint32_t edx = value >> 32;

        asm volatile(
            "wrmsr" 
            :: "c"(msr), "d"(edx), "a"(eax)
        );
    }

    inline cpu* local_core() {
        return reinterpret_cast<cpu*>(rdmsr(GSBase));
    }
}

#endif