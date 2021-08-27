#include "idt.hpp"
#include <stdint.h>
#include <stddef.h>
#include <video.hpp>
#include <memory.hpp>

//TODO: configure PIC or maybe APIC

extern "C" void loadIdt(uint64_t);

__INTERRUPT__ void division_by_zero_handler(interrupt_frame* intFrame);
__INTERRUPT__ void double_fault_handler(interrupt_frame* intFrame);
__INTERRUPT__ void general_protection_handler(interrupt_frame* intFrame, uint64_t errCode);

static IDTGate idt[256];

void registerInterruptHandler(size_t index, uint64_t addr, uint8_t gateType, uint8_t ist) {
    idt[index].offset1 = addr & 0xFFFF;
    idt[index].offset2 = (addr >> 16) & 0xFFFF;
    idt[index].offset3 = addr >> 32;
    idt[index].selector = 0x08;
    idt[index].type = gateType;
    idt[index].zero = 0;
    //TODO: ist?
    idt[index].ist = ist;
}

void init_idt() {
    memset(idt, 0, sizeof(idt));

    registerInterruptHandler(0x0, (uint64_t)&division_by_zero_handler, 0x8E, 0);
    registerInterruptHandler(0x8, (uint64_t)&double_fault_handler, 0x8E, 0);
    registerInterruptHandler(0xD, (uint64_t)&general_protection_handler, 0x8E, 0);

    IDTDescriptor idtDescriptor = {sizeof(idt), (uint64_t)&idt};

    loadIdt((uint64_t)&idtDescriptor);
}

// INTERRUPT HANDLERS

__INTERRUPT__ void division_by_zero_handler(interrupt_frame* intFrame) {
    kprint("Division by zero!");
    while(true)
        asm("hlt");
}
__INTERRUPT__ void double_fault_handler(interrupt_frame* intFrame) {
    kprint("Double fault!");
    while(true)
        asm("hlt");
}
__INTERRUPT__ void general_protection_handler(interrupt_frame* intFrame, uint64_t errCode) {
    kprint("General protection exception!");

    kprint("Error code: %x\n", errCode);

    while(true)
        asm("hlt");
}
