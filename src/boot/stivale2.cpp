#include <stdint.h>
#include <stddef.h>
#include "stivale2.h"

static uint8_t stack[4096];

static struct stivale2_header_tag_terminal terminalTag = {
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
        .next = 0
    },

    .flags = 0
};

static struct stivale2_header_tag_framebuffer framebufferTag = {
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
        .next = (uint64_t)&terminalTag
    },
   
    .framebuffer_width  = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp    = 0
};

__attribute__((section(".stivale2hdr"), used))
static struct stivale2_header stivaleHeader = {
    .entry_point = 0,

    .stack = (uintptr_t)stack + sizeof(stack),
 
    .flags = (1 << 1) | (1 << 2),
  
    .tags = (uint64_t)&framebufferTag
};

