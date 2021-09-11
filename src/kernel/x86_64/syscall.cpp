#include <stdint.h>
#include <stddef.h>

#include "cpu.hpp"
#include <utils.hpp>
#include <video.hpp>

void syscall_test(context* regs) {
    kprint("syscall test!!!!\n");
}

extern "C" void syscall_main(context* regs) {
    kprint("Syscall main\n");

    switch (regs->rax) {
        case 0:
            syscall_test(regs);
            break;

        default:
            Toystar::utils::panic("invalid syscall");
    }
}