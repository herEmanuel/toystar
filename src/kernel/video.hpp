#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include <stddef.h>

#include <boot/stivale2.hpp>
#include <strings.hpp>

void videoInit(stivale2_struct_tag_framebuffer* frameBufferTag);

void incrementCursorX(uint16_t amount);

void putPixel(uint16_t x, uint16_t y, uint32_t color);
void rect(uint16_t x, uint16_t y, uint16_t sizeX, uint16_t sizeY, uint32_t color);
void clearScreen(uint32_t color);

void printChar(char c, uint32_t color);

void kprint(const char* msg);
void kprint(size_t num);
void kprint(size_t num, size_t base);

template<typename T, typename... Args>
void kprint(const char* msg, T value, Args... args) {
    size_t i = 0;
    
    while (msg[i]) {
        if (msg[i] == '%') {
            switch (msg[++i]) {
                case 'd':
                case 'i':
                    kprint(value, 10);
                    break;

                case 's':
                    kprint(value);
                    break;

                case 'x':
                    kprint(value, 16);
                    break;

                case '%':
                    printChar('%', 0xffffff);
                    break;

                default:
                    break;
            }
            i++;
            
            kprint(msg + i, args...);
            return;
        }

        printChar(msg[i], 0xffffff);
        i++;
    }
}

void log(const char* msg);

template<typename T, typename... Args>
void log(const char* msg, T value, Args... args) {
    printChar('[', 0xF8F0FB);
    printChar('I', 0x6320EE);
    printChar('N', 0x6320EE);
    printChar('F', 0x6320EE);
    printChar('O', 0x6320EE);
    printChar(']', 0xF8F0FB);
    printChar(' ', 0x0);

    kprint(msg, value, args...);
}

#endif
