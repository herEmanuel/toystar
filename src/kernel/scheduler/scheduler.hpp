#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stddef.h>
#include <vector.hpp>
#include <lock.hpp>
#include <memory/vmm.hpp>
#include <fs/vfs.hpp>

#define USER_STACK 0x000000800000000
#define USER_STACK_SIZE PAGE_SIZE * 8

struct context;

namespace Sched {
    
    struct process;

    struct thread {
        size_t tid;
        size_t status;

        process* parent_process;
        context* regs;
    };

    struct process {
        size_t pid;
        size_t status;

        Vfs::fs_node* process_directory;

        toys::vector<thread*> threads;
        toys::vector<Vfs::file_description*> fd_list;
        VMM::vmm* pagemap;
    };

    enum Status {
        Waiting, 
        Running,
        Dying,
        Dead
    };

    extern "C" {
        void context_switch(context* regs);
        void reschedule(context* regs);
    }

    void init();
    void queue(thread* thread_to_queue);
    process* create_process(uint64_t rip, uint64_t cs, VMM::vmm* pagemap, Vfs::fs_node* dir);
    bool exit_process();
    process* create_program(const char* path, const char** argv);

}

void syscall_getpid(context* regs);
void syscall_gettid(context* regs);
void syscall_exit(context* regs);
void syscall_fork(context* regs);

#endif