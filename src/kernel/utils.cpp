#include "utils.hpp"
#include <video.hpp>
#include <x86_64/cpu.hpp>
#include <assert.hpp>
#include <strings.hpp>

namespace Toystar::utils {

    void panic(const char* msg) {
        asm("cli");
        
        clearScreen(0xff1340);
        
        kprint("Kernel panic: %s\n", msg);

        Cpu::halt();
    }

}

void __assert(const char* msg, const char* file, size_t line) {
    asm("cli");
    
    kprint("Assertation failed at %s, line %d: %s\n", file, line, msg);

    Cpu::halt();
}
