#include "files.hpp"
#include "vfs.hpp"
#include <x86_64/cpu.hpp>
#include <scheduler/scheduler.hpp>

void syscall_open(context* regs) {
    auto fd = Vfs::open((const char*)regs->rdi, (uint16_t)regs->rsi);

    if (fd == nullptr) {
        regs->rax = 1;
        return;
    }

    cpu* running_proc = Cpu::local_core();

    Sched::process* proc = Sched::get_proces(running_proc->pid);
    
    for (size_t i = 0; i < proc->fd_list.size(); i++) {
        if (proc->fd_list[i] == nullptr) {
            proc->fd_list[i] = fd;
            regs->rax = i;
            return;
        }
    }

    proc->fd_list.push_back(fd);
    regs->rax = proc->fd_list.size() - 1;
}

void syscall_read(context* regs) {
    cpu* running_proc = Cpu::local_core();

    Sched::process* proc = Sched::get_proces(running_proc->pid);

    if (regs->rdi >= proc->fd_list.size()) {
        regs->rax = 1;
        return;
    }

    auto fd = proc->fd_list[regs->rdi];

    if (fd == nullptr) {
        regs->rax = 1;
        return;
    }

    if (!(fd->mode & Vfs::Modes::READ)) {
        regs->rax = 1;
        return;
    }

    regs->rax = Vfs::read(fd, fd->offset, regs->rdx, (const char*)regs->rsi);
}

void syscall_write(context* regs) {
    cpu* running_proc = Cpu::local_core();

    Sched::process* proc = Sched::get_proces(running_proc->pid);

    if (regs->rdi >= proc->fd_list.size()) {
        regs->rax = 1;
        return;
    }

    auto fd = proc->fd_list[regs->rdi];

    if (fd == nullptr) {
        regs->rax = 1;
        return;
    }

    if (!(fd->mode & Vfs::Modes::WRITE)) {
        regs->rax = 1;
        return;
    }

    regs->rax = Vfs::write(fd, fd->offset, regs->rdx, (char*)regs->rsi);
}

void syscall_close(context* regs) {
    cpu* running_proc = Cpu::local_core();

    Sched::process* proc = Sched::get_proces(running_proc->pid);

    if (regs->rdi >= proc->fd_list.size()) {
        regs->rax = 1;
        return;
    }

    if (proc->fd_list[regs->rdi] != nullptr) {
        delete proc->fd_list[regs->rdi];
        proc->fd_list[regs->rdi] = nullptr;
    }

    regs->rax = 0;
}