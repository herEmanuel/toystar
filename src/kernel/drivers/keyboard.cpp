#include "keyboard.hpp"
#include <io.hpp>
#include <video.hpp>
#include <x86_64/idt.hpp>
#include <x86_64/apic.hpp>
#include <strings.hpp>

#include <stdint.h>

/* PS/2 keyboard driver */

static State state = {
    shift: false,
    caps: false
};

__INTERRUPT__ void keyboard_handler(interrupt_frame* frame) {
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