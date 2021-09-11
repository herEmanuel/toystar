#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include <stddef.h>

#include <boot/stivale2.hpp>
#include <strings.hpp>

void video_init(stivale2_struct_tag_framebuffer* frameBufferTag);

void increment_cursor_x(uint16_t amount);

void put_pixel(uint16_t x, uint16_t y, uint32_t color);
void rect(uint16_t x, uint16_t y, uint16_t sizeX, uint16_t sizeY, uint32_t color);
void clear_screen(uint32_t color);

void print_char(char c, uint32_t color);

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
                    print_char('%', 0xffffff);
                    break;

                default:
                    break;
            }
            i++;
            
            kprint(msg + i, args...);
            return;
        }

        print_char(msg[i], 0xffffff);
        i++;
    }
}

void log(const char* msg);

template<typename T, typename... Args>
void log(const char* msg, T value, Args... args) {
    print_char('[', 0xF8F0FB);
    print_char('I', 0x6320EE);
    print_char('N', 0x6320EE);
    print_char('F', 0x6320EE);
    print_char('O', 0x6320EE);
    print_char(']', 0xF8F0FB);
    print_char(' ', 0x0);

    kprint(msg, value, args...);
}

#endif
