#include "gdt.hpp"
#include <stdint.h>

extern "C" void loadGdt(uint64_t);

static GDT gdt;
static GDTDescriptor gdtDescriptor;

void init_gdt() {
    gdt.null = {0, 0, 0, 0, 0, 0};
    gdt.kernelCode = {0, 0, 0, 0x9A, 0x20, 0};
    gdt.kernelData = {0, 0, 0, 0x92, 0, 0};
    gdt.userCode = {0, 0, 0, 0xFA, 0x20, 0};
    gdt.userData = {0, 0, 0, 0xF2, 0, 0};
    gdt.tss = {104, 0, 0, 0x89, 0, 0, 0, 0};

    gdtDescriptor = {sizeof(gdt) - 1, (uint64_t)&gdt};
}

void load_gdt() {
    loadGdt((uint64_t)&gdtDescriptor);
}

void load_tss(uint64_t tssAddr) {
    gdt.tss.base1 = tssAddr & 0xFFFF;
    gdt.tss.base2 = (tssAddr >> 16) & 0xFF;
    gdt.tss.base3 = (tssAddr >> 24) & 0xFF;
    gdt.tss.base4 = tssAddr >> 32;

    asm volatile("ltr %0" :: "r"((uint16_t)0x28));
}