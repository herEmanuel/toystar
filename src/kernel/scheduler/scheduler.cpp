#include "scheduler.hpp"

#include <stdint.h>
#include <stddef.h>
#include <queue.hpp>
#include <vector.hpp>
#include <map.hpp>
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
#include <bitmap.hpp>
#include <video.hpp>

Lock::lock_t sched_lock = 0;

toys::vector<Sched::process*> process_list;
toys::queue<Sched::thread*> waiting_queue;

uint8_t* pid_bitmap = nullptr;
uint8_t* tid_bitmap = nullptr;

Sched::process* init_process = nullptr;

void init_proc() {
    kprint("Main process started\n");
    
    Cpu::hang();
}

namespace Sched {

    void init() {
        pid_bitmap = reinterpret_cast<uint8_t*>(PMM::alloc(1));
        tid_bitmap = reinterpret_cast<uint8_t*>(PMM::alloc(1));

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
        return toys::Bitmap::allocate(pid_bitmap);
    }

    size_t get_new_tid() {
        return toys::Bitmap::allocate(tid_bitmap);
    }

    process* create_process(uint64_t rip, uint64_t cs, VMM::vmm* pagemap) {
        process* new_process = new process;

        new_process->status = Status::Waiting;
        new_process->pagemap = pagemap;

        thread* main_thread = new thread;

        main_thread->status = Status::Waiting;
        main_thread->regs = new context;

        memset(main_thread->regs, 0, sizeof(context));

        uint64_t stack = (uint64_t) PMM::alloc(USER_STACK_SIZE / PAGE_SIZE);
        
        if (cs & 0x3) {
            //userland process
            pagemap->map_range(USER_STACK, stack, USER_STACK_SIZE, 0b111, 0);

            main_thread->regs->rsp = USER_STACK + USER_STACK_SIZE;
            main_thread->regs->ss = 0x20 | 0x3; // | 0x3 sets the RPL
        } else {
            //kernel process
            main_thread->regs->rsp = stack + PHYSICAL_BASE_ADDRESS;
            main_thread->regs->ss = 0x10;
        }

        main_thread->regs->rip = rip;
        main_thread->regs->rflags = 0x202;
        main_thread->regs->cs = cs;
        new_process->threads.push_back(main_thread);

        Lock::acquire(&sched_lock);

        new_process->pid = get_new_pid();
        main_thread->tid = get_new_tid();
        main_thread->parent_process = new_process;

        process_list.push_back(new_process);

        Lock::release(&sched_lock);
        
        return new_process;  
    }

    void queue(thread* thread_to_queue) {
        Lock::acquire(&sched_lock);
        
        waiting_queue.push(thread_to_queue);

        Lock::release(&sched_lock);
    }
    
    extern "C" void reschedule(context* regs) {
        cpu* current_core = Cpu::local_core();

        Lock::acquire(&sched_lock);
        
        thread* scheduled_thread = waiting_queue.pop().value_or(nullptr);
    
        if (scheduled_thread == nullptr) {
            if (current_core->running_thread != nullptr) {
                scheduled_thread = current_core->running_thread;
            } else {
                Lock::release(&sched_lock);
                Apic::localApic->eoi();
                return;
            }
        }
        
        if (current_core->running_process != nullptr && current_core->running_thread != nullptr) {
            thread* previous_thread = current_core->running_thread;

            previous_thread->status = Status::Waiting;
            previous_thread->regs = regs;

            process* previous_process = current_core->running_process;
            previous_process->status = Status::Waiting;

            for (size_t i = 0; i < previous_process->threads.size(); i++) {
                if (previous_process->threads[i] != nullptr 
                    && previous_process->threads[i]->status == Status::Running) 
                    {
                        previous_process->status = Status::Running;
                    }
            }

            // S-L-O-W A-F
            waiting_queue.push(previous_thread);
        }
        
        process* parent_process = scheduled_thread->parent_process;
        parent_process->status = Status::Running;

        scheduled_thread->status = Status::Running;
        
        Lock::release(&sched_lock);
    
        current_core->running_thread = scheduled_thread;
        current_core->running_process = parent_process;
        current_core->pagemap = parent_process->pagemap;
        current_core->working_dir = parent_process->process_directory;
        
        parent_process->pagemap->switch_pagemap();
        
        Apic::localApic->eoi();
        
        context_switch(scheduled_thread->regs);
    }
}

void syscall_getpid(context* regs) {
    cpu* current = Cpu::local_core();

    regs->rax = current->running_process->pid;
}

void syscall_gettid(context* regs) {
    cpu* current = Cpu::local_core();

    regs->rax = current->running_thread->tid;
}

void syscall_exit(context* regs) {
    cpu* current_core = Cpu::local_core();

    Lock::acquire(&sched_lock);

    Sched::process* proc = current_core->running_process;
    
    for (size_t i = 0; i < process_list.size(); i++) {
        if (process_list[i]->pid == proc->pid) 
            process_list.erase(i);
    }
    
    for (size_t i = 0; i < proc->fd_list.size(); i++) {
        delete proc->fd_list[i];
    }

    uint64_t stack_addr = proc->pagemap->virtual_to_physical(USER_STACK);
    PMM::free((void*)stack_addr, USER_STACK_SIZE/PAGE_SIZE);
    
    //TODO: delete user pagemap

    //thats gonna be slow af
    for (size_t i = 0; i < proc->threads.size(); i++) {
        if (proc->threads[i]->status == Sched::Status::Running) 
            continue;

        for (size_t t = 0; t < waiting_queue.size(); t++) {
            if (waiting_queue[t]->tid == proc->threads[i]->tid)
                waiting_queue.erase(t);
        }

        toys::clear_bit(tid_bitmap, proc->threads[i]->tid);
        delete proc->threads[i];
    }

    toys::clear_bit(pid_bitmap, proc->pid);
    delete proc;

    Lock::release(&sched_lock);

    current_core->running_thread = nullptr;
    current_core->running_process = nullptr;
 
    Cpu::hang();
}