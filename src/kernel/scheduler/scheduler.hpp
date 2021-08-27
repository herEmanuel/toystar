#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stddef.h>
#include <vector.hpp>
#include <lock.hpp>
#include <x86_64/cpu.hpp>
#include <memory/vmm.hpp>

struct thread {
    size_t tid;
    size_t parent_pid;
    size_t status;
    size_t waiting_time;

    uint64_t kernel_stack;
    uint64_t user_stack;

    context* regs;
};

struct process {
    size_t pid;
    size_t status;
    size_t waiting_time;

    toys::vector<thread*> threads;
    VMM::Pagemap* pagemap;
};

enum Status {
    Waiting, 
    Running
};

extern "C" {
    void context_switch(context* regs);
    void reschedule(context* regs);
}

void scheduler_init();
process* create_process(uint64_t rip);


#endif