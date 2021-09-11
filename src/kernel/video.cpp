#include "video.hpp"
#include "font.hpp"
#include <strings.hpp>
#include <memory.hpp>
#include <stdint.h>
#include <boot/stivale2.hpp>

static uint32_t* frameBuffer;
static uint16_t pitch;
static uint16_t bpp;
static uint16_t screen_width;
static uint16_t screen_height;
static uint16_t cursor_x;
static uint16_t cursor_y;

void increment_cursor_x(uint16_t amount) {
    cursor_x += amount;
    if (cursor_x >= screen_width) {
        cursor_x = 0;
        cursor_y += 18;

        if (cursor_y >= screen_height) {
            clear_screen(0x0);
        }

    }
}

void put_pixel(uint16_t x, uint16_t y, uint32_t color) {
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

void clear_screen(uint32_t color) {
    rect(0, 0, screen_width, screen_height, color);
}

void video_init(stivale2_struct_tag_framebuffer* frameBufferTag) {
    frameBuffer = (uint32_t*)frameBufferTag->framebuffer_addr;
    pitch = frameBufferTag->framebuffer_pitch;
    bpp = frameBufferTag->framebuffer_bpp;
    screen_width = frameBufferTag->framebuffer_width;
    screen_height = frameBufferTag->framebuffer_height;

    clear_screen(0x8075FF);
}

void print_char(char c, uint32_t color) {

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
                frameBuffer[(b + cursor_y) * (pitch / sizeof(uint32_t)) + j + cursor_x] = color;
            }
        }
    }

    increment_cursor_x(10);
}

void kprint(const char* msg) {
    size_t i = 0;

    while (msg[i]) {
        print_char(msg[i], 0xffffff);
        i++;
    }
}

void kprint(size_t num) {
    const char* msg = itoa(num, 16);
    kprint(msg);
}

void kprint(size_t num, size_t base) {
    const char* msg = itoa(num, base);
    kprint(msg);
}

void log(const char* msg) {
    print_char('[', 0xF8F0FB);
    print_char('I', 0x6320EE);
    print_char('N', 0x6320EE);
    print_char('F', 0x6320EE);
    print_char('O', 0x6320EE);
    print_char(']', 0xF8F0FB);
    print_char(' ', 0x0);

    kprint(msg);
}