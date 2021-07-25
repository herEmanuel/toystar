#include <stdint.h>
#include <stddef.h>
#include <boot/stivale2.h>
#include "vga.h"

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

    // uint32_t* buffer = (uint32_t*)frameBufferInfo->framebuffer_addr;

    // for (int i = 0; i < frameBufferInfo->framebuffer_height; i++) {
    //     for (int c = 0; c < frameBufferInfo->framebuffer_width; c++) {
    //         buffer[i * (frameBufferInfo->framebuffer_pitch / 4) + c] = 0x9061ff;
    //     }
    // }

    kprint("teste");

    while(true)
        asm ("hlt");
}
