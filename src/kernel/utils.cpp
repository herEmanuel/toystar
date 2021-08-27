#include "utils.hpp"
#include <video.hpp>
#include <x86_64/cpu.hpp>

namespace Toystar::utils {

    void panic(const char* msg) {
        asm("cli");
        
        clearScreen(0xff1340);
        
        kprint("Kernel panic: %s\n", msg);

        Cpu::halt();
    }

}