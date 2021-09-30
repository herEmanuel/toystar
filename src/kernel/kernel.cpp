#include <boot/stivale2.hpp>
#include "x86_64/gdt.hpp"
#include "x86_64/idt.hpp"
#include "x86_64/apic.hpp"
#include "x86_64/cpu.hpp"
#include "memory/pmm.hpp"
#include "memory/vmm.hpp"
#include "memory/heap.hpp"
#include "acpi/acpi.hpp"
#include "video.hpp"
#include "pci.hpp"
#include "drivers/keyboard.hpp"
#include "drivers/hpet.hpp"
#include "drivers/serial.hpp"
#include "scheduler/scheduler.hpp"
#include <fs/vfs.hpp>
#include <fs/tmpfs.hpp>
#include <fs/inittmpfs.hpp>

#include <stdint.h>
#include <stddef.h>
#include <strings.hpp>
#include <memory.hpp>
#include <optional.hpp>

void* get_tag(stivale2_struct* first_tag, uint64_t tag_id) {
    stivale2_tag *current_tag = (stivale2_tag*)first_tag->tags;

    while (true) {
        if (!current_tag) {
            return nullptr;
        }

        if (current_tag->identifier == tag_id) {
            return current_tag;
        }

        current_tag = (stivale2_tag*)current_tag->next;
    }
}

extern "C" void _init();

extern "C" void _start(stivale2_struct* stivale2) {
    stivale2_struct_tag_framebuffer* fb_info;
    fb_info = (stivale2_struct_tag_framebuffer*)get_tag(stivale2, STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);

    stivale2_struct_tag_memmap* memmap_info;
    memmap_info = (stivale2_struct_tag_memmap*)get_tag(stivale2, STIVALE2_STRUCT_TAG_MEMMAP_ID);

    stivale2_struct_tag_rsdp* rsdp;
    rsdp = (stivale2_struct_tag_rsdp*)get_tag(stivale2, STIVALE2_STRUCT_TAG_RSDP_ID);

    stivale2_struct_tag_smp* smp_info;
    smp_info = (stivale2_struct_tag_smp*)get_tag(stivale2, STIVALE2_STRUCT_TAG_SMP_ID);

    stivale2_struct_tag_modules* modules_info;
    modules_info = (stivale2_struct_tag_modules*)get_tag(stivale2, STIVALE2_STRUCT_TAG_MODULES_ID);

    Serial::init();
    video_init(fb_info);
    
    print("Kernel loaded\n");

    print("Cores: %d\n", smp_info->cpu_count);

    init_gdt();
    load_gdt();

    init_idt();
    load_idt();
    print("IDT and GDT loaded\n");

    PMM::init(memmap_info->memmap, memmap_info->entries);

    Heap::init();
    
    VMM::init();

    _init();

    print("VMM and PMM initialized\n");

    VMM::vmm* new_vmm = new VMM::vmm(true);

    // new_vmm->map_range_raw(0, 0, 0x100000000, 0b111);

    // log("frames used: %d\n", new_vmm->page_tables_addr.size());

    // PCI::enumerateDevices();

    Acpi::init(rsdp);

    Hpet::init();
    Keyboard::init();

    Apic::init();

    // print("Apic initialized\n");

    // print("Free memory: %d KB\n", PMM::get_available_memory()/1024);

    Tmpfs::init();

    Vfs::mount("tmpfs", "/");
    
    // print("Mounted tmpfs at /\n");
    
    Tmpfs::load(modules_info);

    // print("Initrd loaded\n");

    Cpu::bootstrap_cores(smp_info);

    // print("Initializing scheduler\n");

    Sched::init();

    Cpu::hang();
}
