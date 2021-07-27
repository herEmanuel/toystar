#include <stdint.h>
#include <stddef.h>
#include <boot/stivale2.h>
#include <libc/strings.h>
#include <libc/memory.h>
#include "vga.h"
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

    video_init(frameBufferInfo);
    kprint("Kernel loaded!\n");
    kprint("---RESOLUTION---\n");
    kprint("Width: ");
    kprint(itoa(frameBufferInfo->framebuffer_width));
    kprint("\n");
    kprint("Height: ");
    kprint(itoa(frameBufferInfo->framebuffer_height));
    kprint("\n");

    init_gdt();
    kprint("GDT loaded\n");

    init_idt();
    kprint("IDT loaded\n");

    asm volatile(
        "movq $100, %rax\n"
        "xor %rcx, %rcx\n"
        "idiv %rcx"
    );

    while(true)
        asm ("hlt");
}
