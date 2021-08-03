#include <stdint.h>
#include <stddef.h>
#include <boot/stivale2.h>
#include <libc/strings.h>
#include <libc/memory.h>
#include "vga.h"
#include "paging/pmm.h"
#include "x86_64/gdt.h"
#include "x86_64/idt.h"
#include "drivers/keyboard.h"

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

    registerInterruptHandler(0x21, (uint64_t)&keyboard_handler, 0x8E, 0);

    PMM::init(memmapInfo->memmap, memmapInfo->entries);

    while(true)
        asm ("hlt");
}
