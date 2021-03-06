#ifndef GDT_H
#define GDT_H

#include <stdint.h>

struct GDTDescriptor {
    uint16_t limit;
    uint64_t offset;
} __attribute__((packed));

struct GDTEntry {
    uint16_t limit1;
    uint16_t base1;
    uint8_t base2;
    uint8_t access;
    uint8_t flags;
    uint8_t base3;
} __attribute__((packed));

struct TSSEntry {
    uint16_t limit;
    uint16_t base1;
    uint8_t base2;
    uint8_t flags1;
    uint8_t flags2;
    uint8_t base3;
    uint32_t base4;
    uint32_t reserved;
} __attribute__((packed));

struct GDT {
    GDTEntry null;
    GDTEntry kernelCode;
    GDTEntry kernelData;
    GDTEntry userCode;
    GDTEntry userData;
    TSSEntry tss;
} __attribute__((packed));

void init_gdt();
void load_gdt();

void load_tss(uint64_t tssAddr);

#endif