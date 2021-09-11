#include "idt.hpp"
#include "cpu.hpp"
#include <stdint.h>
#include <stddef.h>
#include <video.hpp>
#include <memory.hpp>
#include <x86_64/apic.hpp>
#include <x86_64/cpu.hpp>

extern "C" {

    void isr_div_by_zero(interrupt_frame* iframe);
    void isr_breakpoint(interrupt_frame* iframe);
    void isr_double_fault(interrupt_frame* iframe);
    void isr_general_protection(interrupt_frame* iframe, uint64_t errCode);

    void _isr_div_by_zero();
    void _isr_breakpoint();
    void _isr_double_fault();
    void _isr_general_protection();

    void reschedule_handler();
    void syscall_entry();

}

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

    registerInterruptHandler(0x0, (uint64_t)&_isr_div_by_zero, 0x8E, 0);
    registerInterruptHandler(0x3, (uint64_t)&_isr_breakpoint, 0x8E, 0);
    registerInterruptHandler(0x8, (uint64_t)&_isr_double_fault, 0x8E, 0);
    registerInterruptHandler(0xD, (uint64_t)&_isr_general_protection, 0x8E, 0);
    registerInterruptHandler(0x20, (uint64_t)&reschedule_handler, 0x8E, 0);
    registerInterruptHandler(0x80, (uint64_t)&syscall_entry, 0xEE, 0);

    idtDescriptor = {sizeof(idt), (uint64_t)&idt};
}

void load_idt() {
    asm volatile("lidt (%0)" :: "r"(&idtDescriptor));
}

extern "C" {

    void isr_div_by_zero(interrupt_frame* iframe) {
        kprint("Division by zero!");
        
        Cpu::halt();
    }

    void isr_breakpoint(interrupt_frame* iframe) {
        kprint("Breakpoint");
        
        Cpu::halt();
    }

    void isr_double_fault(interrupt_frame* iframe) {
        kprint("Double fault!");
        
        Cpu::halt();
    }

    void isr_general_protection(interrupt_frame* iframe, uint64_t errCode) {
        kprint("General protection exception!");

        kprint("Error code: %x\n", errCode);

        Cpu::halt();
    }

}
