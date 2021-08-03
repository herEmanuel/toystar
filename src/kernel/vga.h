#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include <stddef.h>
#include <boot/stivale2.h>

void video_init(stivale2_struct_tag_framebuffer* frameBufferTag);

void incrementCursorX(uint16_t amount);

void putPixel(uint16_t x, uint16_t y, uint32_t color);
void rect(uint16_t x, uint16_t y, uint16_t sizeX, uint16_t sizeY, uint32_t color);

void printChar(char c, uint32_t color);
void kprint(const char* msg);
void kprint(size_t num);

#endif
