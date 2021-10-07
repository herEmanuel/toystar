#include <stdint.h>
#include <stddef.h>

#include <scheduler/scheduler.hpp>
#include <fs/files.hpp>
#include "cpu.hpp"
#include <utils.hpp>
#include <video.hpp>

typedef void (*syscall)(context* regs);

void syscall_print(context* regs) {
    print("%s\n", (const char*)regs->rdi);
}

syscall syscall_list[] = {
    &syscall_read, &syscall_write, &syscall_open, &syscall_close, 
    &syscall_getpid, &syscall_gettid, &syscall_exit, &syscall_print, &syscall_fork
};

extern "C" void syscall_main(context* regs) {
    if (regs->rax >= sizeof(syscall_list)/sizeof(syscall)) {
        panic("invalid syscall"); //TODO: treat it properly
    }
    
    syscall_list[regs->rax](regs);
}