#include "keyboard.hpp"
#include <x86_64/idt.hpp>
#include <x86_64/apic.hpp>
#include <strings.hpp>
#include <video.hpp>
#include <io.hpp>

#include <stdint.h>

/* PS/2 keyboard driver */

extern "C" void _isr_keyboard();

namespace Keyboard {

    static State state = {
        shift: false,
        caps: false
    };

    void init() {
        registerInterruptHandler(0x21, (uint64_t)&_isr_keyboard, 0x8E, 0);  
    }

    extern "C" void isr_keyboard(interrupt_frame* iframe) {
        uint8_t key = inb(0x60);

        if (key >= MAX_SCANCODE) {
            uint8_t releasedKey = key - 0x80;

            switch (releasedKey) {
                case LSHIFT:
                case RSHIFT:
                    state.shift = false;
                    break;
                default:
                    break;
            }

            Apic::localApic->eoi();
            return;
        }

        switch (key) {
            case LSHIFT:
            case RSHIFT:
                state.shift = true;
                Apic::localApic->eoi();
                return;

            case CAPS:
                state.caps = !state.caps;
                Apic::localApic->eoi();
                return;

            default:
                break;
        }

        char buffer;
        
        if (state.shift && state.caps) {
            buffer = scShiftCaps[key];
        } else if (state.shift) {
            buffer = scShift[key];
        } else if (state.caps) {
            buffer = scCaps[key];
        } else {
            buffer = scNormal[key];
        }

        printChar(buffer, 0xffffff);
        Apic::localApic->eoi();
    }

}