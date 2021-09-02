#include "idt.hpp"
#include "cpu.hpp"
#include <stdint.h>
#include <stddef.h>
#include <video.hpp>
#include <memory.hpp>
#include <x86_64/apic.hpp>

__INTERRUPT__ void division_by_zero_handler(interrupt_frame* intFrame);
__INTERRUPT__ void breakpoint_handler(interrupt_frame* intFrame);
__INTERRUPT__ void double_fault_handler(interrupt_frame* intFrame);
__INTERRUPT__ void general_protection_handler(interrupt_frame* intFrame, uint64_t errCode);

extern "C" void reschedule_handler();

static IDTGate idt[256];
static IDTDescriptor idtDescriptor;

void registerInterruptHandler(size_t index, uint64_t addr, uint8_t gateType, uint8_t ist) {
    idt[index].offset1 = addr & 0xFFFF;
    idt[index].offset2 = (addr >> 16) & 0xFFFF;
    idt[index].offset3 = addr >> 32;
    idt[index].selector = 0x08;
    idt[index].type = gateType;
    idt[index].zero = 0;
    idt[index].ist = ist;
}

void init_idt() {
    memset(idt, 0, sizeof(idt));

    registerInterruptHandler(0x0, (uint64_t)&division_by_zero_handler, 0x8E, 0);
    registerInterruptHandler(0x3, (uint64_t)&breakpoint_handler, 0x8E, 0);
    registerInterruptHandler(0x8, (uint64_t)&double_fault_handler, 0x8E, 0);
    registerInterruptHandler(0xD, (uint64_t)&general_protection_handler, 0x8E, 0);
    registerInterruptHandler(0x20, (uint64_t)&reschedule_handler, 0x8E, 0);

    idtDescriptor = {sizeof(idt), (uint64_t)&idt};
}

void load_idt() {
    asm volatile("lidt (%0)" :: "r"(&idtDescriptor));
}

// INTERRUPT HANDLERS

__INTERRUPT__ void division_by_zero_handler(interrupt_frame* intFrame) {
    kprint("Division by zero!");
    
    Cpu::halt();
}

__INTERRUPT__ void breakpoint_handler(interrupt_frame* intFrame) {
    kprint("Breakpoint");
    
    Cpu::halt();
}

__INTERRUPT__ void double_fault_handler(interrupt_frame* intFrame) {
    kprint("Double fault!");
    
    Cpu::halt();
}
__INTERRUPT__ void general_protection_handler(interrupt_frame* intFrame, uint64_t errCode) {
    kprint("General protection exception!");

    kprint("Error code: %x\n", errCode);

    Cpu::halt();
}
