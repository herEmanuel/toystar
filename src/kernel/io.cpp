#include "io.hpp"

uint8_t inb(uint16_t port) {
    uint8_t byte;

    asm (
        "movw %1, %%dx\n"
        "inb %%dx, %0"
         : "=r"(byte)
         : "r"(port) 
         : "rdx"
    );

    return byte;
}

uint16_t inw(uint16_t port) {
    uint16_t byte;

    asm (
        "movw %1, %%dx\n"
        "inw %%dx, %0"
         : "=r"(byte)
         : "r"(port) 
         : "rdx"
    );

    return byte;
}

uint32_t inl(uint16_t port) {
    uint32_t byte;

    asm (
        "movw %1, %%dx\n"
        "inl %%dx, %0"
         : "=r"(byte)
         : "r"(port) 
         : "rdx"
    );

    return byte;
}

void outb(uint16_t port, uint8_t value) {
    asm (
        "movb %1, %%al\n"
        "outb %%al, %0"
         : : "r"(port), "r"(value)
         : "rax"
    );
}

void outw(uint16_t port, uint16_t value) {
    asm (
        "movw %1, %%ax\n"
        "outw %%ax, %0"
         : : "r"(port), "r"(value)
         : "rax"
    );
}

void outl(uint16_t port, uint32_t value) {
    asm (
        "movl %1, %%eax\n"
        "outl %%eax, %0"
         : : "r"(port), "r"(value)
         : "rax"
    );
}