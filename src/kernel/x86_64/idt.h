#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define __INTERRUPT__ __attribute__((interrupt))

struct interrupt_frame;

struct IDTDescriptor {
    uint16_t limit;
    uint64_t offset;
} __attribute__((packed));

struct IDTGate {
    uint16_t offset1;
    uint16_t selector;
    uint8_t ist;
    uint8_t type;
    uint16_t offset2;
    uint32_t offset3;
    uint32_t zero;
} __attribute__((packed));  

IDTGate encodeIDTGate(uint64_t addr, uint8_t gateType, uint16_t cSelector, uint8_t ist);
void init_idt();

#endif