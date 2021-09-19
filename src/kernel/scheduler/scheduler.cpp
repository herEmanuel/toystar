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
#include <fs/vfs.hpp>
#include <fs/tmpfs.hpp>
#include <loader/elf.hpp>

Lock::lock_t sched_lock = 0;

toys::vector<Sched::process*> process_list;
toys::vector<Sched::thread*> thread_list;

Sched::process* init_process = nullptr;

void init_proc() {
    kprint("Main process started\n");
    
    Cpu::halt();
}

namespace Sched {

    void init() {
        // init_process = create_process((uint64_t)&init_proc, 0x8, VMM::kernel_vmm);
        // queue(init_process->threads[0]);
   
        auto fd = Vfs::open("/hello.elf", 0);
        if (fd == nullptr) {
            kprint("could not open the elf :(\n");
        }

        VMM::vmm* pagemap = new VMM::vmm(true);

        pagemap->map_range_raw(PHYSICAL_BASE_ADDRESS, 0, 0x100000000, 0b11);
        pagemap->map_range_raw(KERNEL_BASE, 0, 0x80000000, 0b11);
        pagemap->map_range_raw(0, 0, 0x100000000, 0b111); //TODO: hahahaha
        
        int64_t entry_point = Loader::Elf::load(pagemap, fd->file);
            
        Sched::process* elf_loaded = Sched::create_process(entry_point, 0x1b, pagemap);
        
        Sched::queue(elf_loaded->threads[0]);

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
        mainThread->regs = new context;

        memset(mainThread->regs, 0, sizeof(context));

        uint64_t stack = (uint64_t) PMM::alloc(USER_STACK_SIZE / PAGE_SIZE);
        
        //TODO: take a look at this
        if (cs & 0x3) {
            //userland process
            pagemap->map_range(USER_STACK, stack, USER_STACK_SIZE, 0b111, 0);

            mainThread->user_stack = USER_STACK + USER_STACK_SIZE;
            mainThread->regs->rsp = mainThread->user_stack;
            kprint("rsp: %x\n", mainThread->regs->rsp);
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

    process* get_proces(size_t pid) {
        Lock::acquire(&sched_lock);

        if (pid >= process_list.size()) {
            return nullptr;
        }

        process* proc = process_list[pid];

        Lock::release(&sched_lock);
        return proc;
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
            if (thread_list[i] == nullptr || thread_list[i]->status == Status::Running) 
                continue;

            thread_list[i]->waiting_time++;
            
            if (thread_list[i]->waiting_time >= waiting_amount) {
                waiting_amount = thread_list[i]->waiting_time;
                thread_id = thread_list[i]->tid;
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

            process* previous_process = process_list[current_core->pid];
            previous_process->status = Status::Waiting;

            for (size_t i = 0; i < previous_process->threads.size(); i++) {
                if (previous_process->threads[i] != nullptr 
                    && previous_process->threads[i]->status == Status::Running) 
                    {
                        previous_process->status = Status::Running;
                    }
            }

        }

        process* parent_process = process_list[scheduled_thread->parent_pid];
        parent_process->status = Status::Running;

        scheduled_thread->status = Status::Running;
        scheduled_thread->waiting_time = 0;

        Lock::release(&sched_lock);
    
        current_core->tid = scheduled_thread->tid;
        current_core->pid = parent_process->pid;
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