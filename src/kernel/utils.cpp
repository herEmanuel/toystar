#include "utils.hpp"
#include <video.hpp>
#include <x86_64/cpu.hpp>
#include <assert.hpp>
#include <strings.hpp>

namespace Toystar::utils {

    void panic(const char* msg) {
        asm("cli");
        
        kprint("Kernel panic: %s\n", msg);

        Cpu::hang();
    }

}

void __assert(const char* msg, const char* file, size_t line) {
    asm("cli");
    
    kprint("Assertation failed at %s, line %d: %s\n", file, line, msg);

    Cpu::hang();
}
