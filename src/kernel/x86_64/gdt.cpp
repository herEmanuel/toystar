#include "gdt.hpp"
#include <stdint.h>

extern "C" void loadGdt(uint64_t);

static GDT gdt;

void init_gdt() {
    gdt.null = {0, 0, 0, 0, 0, 0};
    gdt.kernelCode = {0, 0, 0, 0x9A, 0x20, 0};
    gdt.kernelData = {0, 0, 0, 0x92, 0, 0};
    gdt.userCode = {0, 0, 0, 0xFA, 0x20, 0};
    gdt.userData = {0, 0, 0, 0xF2, 0, 0};

    GDTDescriptor gdtDescriptor = {sizeof(gdt) - 1, (uint64_t)&gdt};

    loadGdt((uint64_t)&gdtDescriptor);
}