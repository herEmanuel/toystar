#include <boot/stivale2.hpp>
#include "x86_64/gdt.hpp"
#include "x86_64/idt.hpp"
#include "x86_64/apic.hpp"
#include "memory/pmm.hpp"
#include "memory/vmm.hpp"
#include "memory/heap.hpp"
#include "acpi/acpi.hpp"
#include "video.hpp"
#include "pci.hpp"
#include "drivers/keyboard.hpp"

#include <stdint.h>
#include <stddef.h>
#include <strings.hpp>
#include <memory.hpp>
#include <vector.hpp>

void* getTag(stivale2_struct* firstTag, uint64_t tagId) {
    stivale2_tag *currentTag = (stivale2_tag*)firstTag->tags;

    while (true) {
        if (!currentTag) {
            return nullptr;
        }

        if (currentTag->identifier == tagId) {
            return currentTag;
        }

        currentTag = (stivale2_tag*)currentTag->next;
    }
}

extern "C" void _start(stivale2_struct* stivale2) {
    stivale2_struct_tag_framebuffer* frameBufferInfo;
    frameBufferInfo = (stivale2_struct_tag_framebuffer*)getTag(stivale2, STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);

    stivale2_struct_tag_memmap* memmapInfo;
    memmapInfo = (stivale2_struct_tag_memmap*)getTag(stivale2, STIVALE2_STRUCT_TAG_MEMMAP_ID);

    stivale2_struct_tag_rsdp* rsdp;
    rsdp = (stivale2_struct_tag_rsdp*)getTag(stivale2, STIVALE2_STRUCT_TAG_RSDP_ID);

    stivale2_struct_tag_smp* smpInfo;
    smpInfo = (stivale2_struct_tag_smp*)getTag(stivale2, STIVALE2_STRUCT_TAG_SMP_ID);

    videoInit(frameBufferInfo);
    
    kprint("Kernel loaded!\n");
    kprint("---RESOLUTION---\n");
    kprint("Width: %d\n", frameBufferInfo->framebuffer_width);
    kprint("Height: %d\n", frameBufferInfo->framebuffer_height);

    init_gdt();
    kprint("GDT loaded\n");

    init_idt();
    kprint("IDT loaded\n");

    registerInterruptHandler(0x21, (uint64_t)&keyboard_handler, 0x8E, 0);

    PMM::init(memmapInfo->memmap, memmapInfo->entries);
    VMM vmm;
    vmm.init();

    kprint("VMM and PMM initialized\n");

    //TODO: keep testing the vmm

    Heap::init();

    PCI::enumerateDevices();

    kprint("Cores: %d\n", smpInfo->cpu_count);

    Acpi::init(rsdp);

    Apic::init();

    void* hpet = Acpi::findTable("HPET");
    kprint("Hpet: %x\n", (size_t)hpet);

    while(true)
        asm ("hlt");
}

// 1mb 512kb 256kb 128kb 64kb 32kb 16kb 8kb 4kb 2kb 1kb 512b 256b 128b 64b 32b 16b
         