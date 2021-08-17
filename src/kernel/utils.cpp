#include "utils.hpp"
#include <video.hpp>

namespace Toystar::utils {

    void panic(const char* msg) {
        asm("cli");
        
        clearScreen(0xff1340);
        
        kprint("Kernel panic: %s\n", msg);

        while(true)
            asm("hlt");
    }

}