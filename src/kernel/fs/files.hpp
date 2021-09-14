#ifndef FILES_H
#define FILES_H

#include <x86_64/cpu.hpp>

void syscall_open(context* regs);
void syscall_read(context* regs);
void syscall_write(context* regs);
void syscall_close(context* regs);

#endif