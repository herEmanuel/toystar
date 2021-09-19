#include <stdint.h>
#include <stddef.h>

#include <scheduler/scheduler.hpp>
#include <fs/files.hpp>
#include "cpu.hpp"
#include <utils.hpp>
#include <video.hpp>

typedef void (*syscall)(context* regs);

void syscall_print(context* regs) {
    kprint("print: %s\n", (const char*)regs->rdi);
}

syscall syscall_list[] = {
    &syscall_read, &syscall_write, &syscall_open, &syscall_close, 
    &syscall_getpid, &syscall_gettid, &syscall_exit, &syscall_print
};

extern "C" void syscall_main(context* regs) {
    if (regs->rax >= sizeof(syscall_list)/sizeof(syscall)) {
        Toystar::utils::panic("invalid syscall"); //TODO: treat it properly
    }

    syscall_list[regs->rax](regs);
}