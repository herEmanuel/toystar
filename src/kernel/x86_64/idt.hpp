#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include <stddef.h>

struct interrupt_frame {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

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

void register_interrupt_handler(size_t index, uint64_t addr, uint8_t gateType, uint8_t ist);
void init_idt();
void load_idt();

#endif