#include "keyboard.hpp"
#include <kernel/io.hpp>
#include <kernel/video.hpp>
#include <kernel/x86_64/idt.hpp>
#include <kernel/x86_64/pic.hpp>
#include <libc/strings.hpp>

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

        sendEOI(1);
        return;
    }

    switch (key) {
        case LSHIFT:
        case RSHIFT:
            state.shift = true;
            sendEOI(1);
            return;

        case CAPS:
            state.caps = !state.caps;
            sendEOI(1);
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
    sendEOI(1);
}