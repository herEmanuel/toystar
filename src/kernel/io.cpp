#include "io.h"

uint8_t inb(uint16_t port) {
    uint8_t byte;

    asm (
        "movw %1, %%dx\n"
        "inb %%dx, %0"
         : "=r"(byte)
         : "r"(port) 
         : "edx"
    );

    return byte;
}

void outb(uint16_t port, uint8_t value) {
    asm (
        "movb %1, %%al\n"
        "outb %%al, %0"
         : : "r"(port), "r"(value)
         : "eax"
    );
}

void outw(uint16_t port, uint16_t value) {
    asm (
        "movw %1, %%ax\n"
        "outw %%ax, %0"
         : : "r"(port), "r"(value)
         : "eax"
    );
}