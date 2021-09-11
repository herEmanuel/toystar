#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <x86_64/idt.hpp>
#include <stdint.h>

#define MAX_SCANCODE 0x59

namespace Keyboard {
    
    struct State {
        bool shift;
        bool caps;
    };

    enum SpecialKeys {
        LSHIFT = 0x2A, 
        LCONTROL = 0x1D,
        CAPS = 0x3A,
        ENTER = 0x1C,
        BACK = 0x0E,
        RSHIFT = 0x36
    };

    const char scNormal[] = {
        '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-',
        '=', '\0', '\0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', 
        ']', '\0', '\0', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 
        '`', '\0', '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 
        '\0', '\0', '\0', ' '
    };

    const char scCaps[] = {
        '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-',
        '=', '\0', '\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', 
        ']', '\0', '\0', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', 
        '`', '\0', '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 
        '\0', '\0', '\0', ' '
    };

    const char scShift[] = {
        '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 
        '\0', '\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 
        '\0', '\0', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
        '\0', '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?'
    };

    const char scShiftCaps[] = {
        '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 
        '\0', '\0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', 
        '\0', '\0', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '"', '~',
        '\0', '|', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?'
    };

    extern "C" void isr_keyboard(interrupt_frame* iframe);
    void init();
    
}

#endif