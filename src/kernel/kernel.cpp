#include <stdint.h>
#include <stddef.h>
#include <boot/stivale2.h>
#include <libc/strings.h>
#include <libc/memory.h>
#include "vga.h"
#include "paging/pmm.h"
#include "x86_64/gdt.h"
#include "x86_64/idt.h"

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

    video_init(frameBufferInfo);
    kprint("Kernel loaded!\n");
    kprint("---RESOLUTION---\n");
    kprint("Width: ");
    kprint(itoa(frameBufferInfo->framebuffer_width, 10));
    kprint("\n");
    kprint("Height: ");
    kprint(itoa(frameBufferInfo->framebuffer_height, 10));
    kprint("\n");

    init_gdt();
    kprint("GDT loaded\n");

    init_idt();
    kprint("IDT loaded\n");

    PMM::init(memmapInfo->memmap, memmapInfo->entries);
    
    void* teste = PMM::alloc(1);  
    void* teste2 = PMM::alloc(1);  
    PMM::free(teste, 1);
    void* teste3 = PMM::alloc(1);  
    kprint("ended :(\n");

    // for (int i = 0; i < 10; i++) {
    //     kprint((size_t)pmmAlloc(1));
    //     kprint("\n");
    // }

    while(true)
        asm ("hlt");
}
