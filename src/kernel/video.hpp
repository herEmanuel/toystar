#ifndef VIDEO_H
#define VIDEO_H

#include <boot/stivale2.hpp>

#include <stdint.h>
#include <stddef.h>

void video_init(stivale2_struct_tag_framebuffer* frameBufferTag);

void increment_cursor_x(uint16_t amount);

void put_pixel(uint16_t x, uint16_t y, uint32_t color);
void rect(uint16_t x, uint16_t y, uint16_t sizeX, uint16_t sizeY, uint32_t color);
void clear_screen(uint32_t color);

void print_char(char c, uint32_t color);

void print(const char* msg);
void print(size_t num);
void print(size_t num, size_t base);

template<typename T, typename... Args>
void print(const char* msg, T value, Args... args) {
    size_t i = 0;
    
    while (msg[i]) {
        if (msg[i] == '%') {
            switch (msg[++i]) {
                case 'd':
                case 'i':
                    print(value, 10);
                    break;

                case 's':
                    print(value);
                    break;

                case 'x':
                    print(value, 16);
                    break;

                case '%':
                    print_char('%', 0xffffff);
                    break;

                default:
                    break;
            }
            i++;
            
            print(msg + i, args...);
            return;
        }

        print_char(msg[i], 0xffffff);
        i++;
    }
}

#endif
