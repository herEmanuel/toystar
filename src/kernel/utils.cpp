#include "utils.hpp"
#include <video.hpp>
#include <drivers/serial.hpp>
#include <x86_64/cpu.hpp>
#include <assert.hpp>
#include <strings.hpp>

void panic(const char* msg) {
    asm("cli");
    
    print("Kernel panic: %s\n", msg);

    Cpu::hang();
}

void __assert(const char* msg, const char* file, size_t line) {
    asm("cli");
    
    print("Assertation failed at %s, line %d: %s\n", file, line, msg);

    Cpu::hang();
}

void log(const char* msg) {
    size_t i = 0;

    while (msg[i]) {
        Serial::send_char(msg[i]);
        i++;
    }
}

void log(size_t num) {
    const char* msg = itoa(num, 16);
    log(msg);
}

void log(size_t num, size_t base) {
    const char* msg = itoa(num, base);
    log(msg);
}