#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include <stddef.h>

#include <boot/stivale2.hpp>
#include <libc/strings.hpp>

void video_init(stivale2_struct_tag_framebuffer* frameBufferTag);

void incrementCursorX(uint16_t amount);

void putPixel(uint16_t x, uint16_t y, uint32_t color);
void rect(uint16_t x, uint16_t y, uint16_t sizeX, uint16_t sizeY, uint32_t color);

void printChar(char c, uint32_t color);
void kprint(const char* msg);
void kprint(size_t num);

template<typename T>
void print_num(T number, size_t base) {
    const char* msg = itoa(number, base);
    kprint(msg);
}

template<typename T, typename... Args>
void kprint(const char* msg, T value, Args... args) {
    size_t i = 0;
    
    while (msg[i]) {
        if (msg[i] == '%') {
            switch (msg[++i]) {
                case 'd':
                case 'i':
                    kprint(value);
                    // print_num(value, 10);
                    break;

                case 's':
                    kprint(value);
                    break;

                case 'x':
                    kprint(value);
                    // print_num(value, 16);
                    break;

                case '%':
                    printChar('%', 0xffffff);
                    break;

                default:
                    break;
            }
            i++;
        }

        printChar(msg[i], 0xffffff);
        i++;
    }

    // if (cursor_x > 0) { incrementCursorX(10); } 
}

#endif
