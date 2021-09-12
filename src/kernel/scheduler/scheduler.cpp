#include "scheduler.hpp"

#include <stdint.h>
#include <stddef.h>
#include <vector.hpp>
#include <lock.hpp>
#include <x86_64/cpu.hpp>
#include <x86_64/apic.hpp>
#include <memory/vmm.hpp>
#include <memory/pmm.hpp>
#include <memory/heap.hpp>
#include <memory.hpp>

Lock::lock_t sched_lock = 0;

toys::vector<Sched::process*> process_list;
toys::vector<Sched::thread*> thread_list;

Sched::process* init_process = nullptr;

void init_proc() {
    kprint("Main process started!\n");
    asm volatile("movq $0x2, %rax");
    asm volatile("int $0x80");
    while (true);
}

namespace Sched {

    void init() {
        VMM::vmm* newVmm = new VMM::vmm(true);
        newVmm->map_range_raw(PHYSICAL_BASE_ADDRESS, 0, 0x100000000, 0b111);
        newVmm->map_range_raw(KERNEL_BASE, 0, 0x80000000, 0b111);
        newVmm->map_range_raw(0, 0, 0x100000000, 0b111);
 
        init_process = create_process((uint64_t)&init_proc, 0x1b, newVmm);
        queue(init_process->threads[0]);
   
        Apic::localApic->calibrate_timer(30);
    }

    size_t get_new_pid() {
        size_t len = process_list.size();
        for (size_t i = 0; i < len; i++) {
            if (process_list[i] == nullptr) {
                return i;
            }
        }

        process_list.push_back(nullptr);
        return len;
    }

    size_t get_new_tid() {
        size_t len = thread_list.size();
        for (size_t i = 0; i < len; i++) {
            if (thread_list[i] == nullptr) {
                return i;
            }
        }

        thread_list.push_back(nullptr);
        return len;
    }

    process* create_process(uint64_t rip, uint64_t cs, VMM::vmm* pagemap) {
        process* newProcess = new process;

        newProcess->status = Status::Waiting;
        newProcess->pagemap = pagemap;

        thread* mainThread = new thread;

        mainThread->status = Status::Waiting;
        mainThread->waiting_time = 0;

        memset(mainThread->regs, 0, sizeof(context));

        uint64_t stack = (uint64_t) PMM::alloc(USER_STACK_SIZE / PAGE_SIZE);
        
        //TODO: take a look at this
        if (cs & 0x3) {
            //userland process
            pagemap->map_range(USER_STACK, stack, USER_STACK_SIZE, 0b111, 0);

            mainThread->user_stack = USER_STACK + USER_STACK_SIZE;
            mainThread->regs->rsp = mainThread->user_stack;
            mainThread->regs->ss = 0x20 | 0x3; // | 0x3 sets the RPL
        } else {
            //kernel process
            mainThread->regs->rsp = stack + PHYSICAL_BASE_ADDRESS;
            mainThread->regs->ss = 0x10;
        }

        mainThread->regs->rip = rip;
        mainThread->regs->rflags = 0x202;
        mainThread->regs->cs = cs;
        newProcess->threads.push_back(mainThread);

        Lock::acquire(&sched_lock);

        newProcess->pid = get_new_pid();
        mainThread->tid = get_new_tid();
        mainThread->parent_pid = newProcess->pid;

        process_list[newProcess->pid] = newProcess;

        Lock::release(&sched_lock);
        
        return newProcess;  
    }

    void queue(thread* thread_to_queue) {
        Lock::acquire(&sched_lock);
        
        thread_list[thread_to_queue->tid] = thread_to_queue;

        Lock::release(&sched_lock);
    }
    
    extern "C" void reschedule(context* regs) {
        size_t waiting_amount = 0;
        int thread_id = -1;
 
        cpu* current_core = Cpu::local_core();

        Lock::acquire(&sched_lock);
        
        for (size_t i = 0; i < thread_list.size(); i++) {
            if (thread_list[i] == nullptr) 
                continue;

            if (thread_list[i]->status == Status::Running) {
                thread_list[i]->waiting_time++;
                continue;
            }
            
            if (thread_list[i]->waiting_time >= waiting_amount) {
                waiting_amount = thread_list[i]->waiting_time;
                thread_id = thread_list[i]->tid;

                thread_list[i]->waiting_time++;
            }
        }
    
        if (thread_id == -1) {
            if (current_core->tid != -1) {
                thread_id = current_core->tid;
            } else {
                Lock::release(&sched_lock);
                Apic::localApic->eoi();
                return;
            }
        }
        
        thread* scheduled_thread = thread_list[thread_id];
        
        if (current_core->pid != -1 && current_core->tid != -1) {
            thread* previous_thread = thread_list[current_core->tid];

            previous_thread->status = Status::Waiting;
            previous_thread->regs = regs;
        }

        process* parent_process = process_list[scheduled_thread->parent_pid];
        //TODO: process status

        scheduled_thread->status = Status::Running;

        Lock::release(&sched_lock);
    
        current_core->tid = scheduled_thread->tid;
        current_core->pid = scheduled_thread->parent_pid;
        current_core->user_stack = scheduled_thread->user_stack;
        current_core->pagemap = parent_process->pagemap;
        current_core->working_dir = parent_process->process_directory;
        
        parent_process->pagemap->switch_pagemap();
        
        Apic::localApic->eoi();
        
        context_switch(scheduled_thread->regs);
    }
}

void syscall_getpid(context* regs) {
    cpu* current = Cpu::local_core();

    regs->rax = current->pid;
}

void syscall_gettid(context* regs) {
    cpu* current = Cpu::local_core();

    regs->rax = current->tid;
}

void syscall_exit(context* regs) {
    cpu* current_core = Cpu::local_core();

    Lock::acquire(&sched_lock);

    Sched::process* proc = process_list[current_core->pid];

    process_list[proc->pid] = nullptr;

    for (size_t i = 0; i < proc->fd_list.size(); i++) {
        delete proc->fd_list[i];
    }

    //TODO: delete user stack and pagemap

    for (size_t i = 0; i < proc->threads.size(); i++) {
        thread_list[proc->threads[i]->tid] = nullptr;
        delete proc->threads[i];
    }

    delete proc;

    Lock::release(&sched_lock);

    current_core->tid = -1;
    current_core->pid = -1;

    Cpu::halt();
}