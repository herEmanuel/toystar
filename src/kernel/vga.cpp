#include "vga.h"
#include "font.h"
#include <libc/strings.h>
#include <stdint.h>
#include <boot/stivale2.h>

static uint32_t* frameBuffer;
static uint16_t pitch;
static uint16_t bpp;
static uint16_t screen_width;
static uint16_t screen_height;
static uint16_t cursor_x;
static uint16_t cursor_y;

void incrementCursorX(uint16_t amount) {
    cursor_x += amount;
    if (cursor_x >= screen_width) {
        cursor_x = 0;
        cursor_y += 18;
    }
}

void putPixel(uint16_t x, uint16_t y, uint32_t color) {
    if (x >= screen_width || y >= screen_height) { return; }

    frameBuffer[y * (pitch / 4) + x] = color;
}

void rect(uint16_t x, uint16_t y, uint16_t sizeX, uint16_t sizeY, uint32_t color) {
    cursor_x = x;
    cursor_y = y;

    for (int h = 0; h < sizeY; h++) {
        for (int w = 0; w < sizeX; w++) {
            frameBuffer[cursor_y * (pitch / 4) + cursor_x] = color;
            cursor_x++;
        }
        cursor_x = x; 
        cursor_y++;
    }

    cursor_x = 0;
    cursor_y = 0;
}

void video_init(stivale2_struct_tag_framebuffer* frameBufferTag) {
    frameBuffer = (uint32_t*)frameBufferTag->framebuffer_addr;
    pitch = frameBufferTag->framebuffer_pitch;
    bpp = frameBufferTag->framebuffer_bpp;
    screen_width = frameBufferTag->framebuffer_width;
    screen_height = frameBufferTag->framebuffer_height;

    rect(0, 0, screen_width, screen_height, 0x9061ff);
}

void printChar(char c, uint32_t color) {

    switch (c) {
        case '\n':
            cursor_y += 18;
            cursor_x = 0;
            return;

        default:
            break;
    }

    size_t offset = (size_t)c * 16;

    for (int b = 0; b < char_height; b++) {
        for (int j = 0; j < char_width; j++) {
            if ((font[offset + b] >> (7 - j)) & 1) {
                frameBuffer[(b + cursor_y) * (pitch / 4) + j + cursor_x] = color;
            }
        }
    }

    incrementCursorX(10);
}

void kprint(const char* msg) {
    size_t i = 0;

    while (msg[i]) {
        printChar(msg[i], 0xffffff);
        i++;
    }

    if (cursor_x > 0) { incrementCursorX(10); } 
}

void kprint(size_t num) {
    const char* msg = itoa(num);
    kprint(msg);
}