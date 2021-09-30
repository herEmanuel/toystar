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
#include <utils.hpp>
#include <drivers/hpet.hpp>

Lock::lock_t sched_lock = 0;

toys::vector<Sched::process*> process_list;
toys::queue<Sched::thread*> waiting_queue;

uint8_t* pid_bitmap = nullptr;
uint8_t* tid_bitmap = nullptr;

Lock::lock_t pid_lock = 0;
Lock::lock_t tid_lock = 0;

Sched::process* init_process = nullptr;

void init_proc() {
    log("Main process started\n");
    Sched::start_program("/hello.elf", nullptr);

    // int64_t res;
    // asm volatile("mov $0x8, %rax");
    // asm volatile("int $0x80" ::: "rax");
    // asm volatile("mov %%rax, %0" : "=r"(res));
    // log("res: %d\n", res);
    // if (res == 0) {
    //     log("nothing\n");
    //     // while(true) {
    //     //     log("CHILD PROCESS YOOO\n");
    //     //     Hpet::sleep(500);
    //     // }
    // } else {
    //     while(true) {
    //         log("PARENT PROCESS!! %d\n", res);
    //         Hpet::sleep(500);
    //     }
    // }

    Cpu::hang();
}

namespace Sched {

    void init() {
        pid_bitmap = reinterpret_cast<uint8_t*>(PMM::alloc(1) + PHYSICAL_BASE_ADDRESS);
        tid_bitmap = reinterpret_cast<uint8_t*>(PMM::alloc(1) + PHYSICAL_BASE_ADDRESS);

        init_process = create_process((uint64_t)&init_proc, 0x8, VMM::kernel_vmm, nullptr);
        queue(init_process->threads[0]);
   
        Apic::localApic->calibrate_timer(500);
    }

    int64_t get_new_pid() {
        Lock::acquire(&pid_lock);
        int64_t pid = toys::Bitmap::allocate(pid_bitmap);
        Lock::release(&pid_lock);

        return pid;
    }

    int64_t get_new_tid() {
        Lock::acquire(&tid_lock);
        int64_t tid = toys::Bitmap::allocate(tid_bitmap);
        Lock::release(&tid_lock);
        
        return tid;
    }

    process* create_process(uint64_t rip, uint64_t cs, VMM::vmm* pagemap, Vfs::fs_node* dir) {
        process* new_process = new process;

        new_process->status = Status::Waiting;
        new_process->pagemap = pagemap;
        new_process->process_directory = dir;

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

        int64_t new_pid = get_new_pid(); 
        int64_t new_tid = get_new_tid(); 

        if (new_pid == -1 || new_tid == -1) {
            return nullptr;
        }

        new_process->pid = new_pid;
        main_thread->tid = new_tid;
        main_thread->parent_process = new_process;

        process_list.push_back(new_process);

        Lock::release(&sched_lock);
        
        return new_process;  
    }

    //still lacks some stuff
    bool start_program(const char* path, const char** argv) {
        auto fd = Vfs::open(path, 0);
        if (fd == nullptr) {
            return false;
        }

        VMM::vmm* pagemap = new VMM::vmm(true);

        int64_t rip = Loader::Elf::load(pagemap, fd->file);

        process* proc = create_process(rip, 0x1b, pagemap, fd->file->parent); //always userland program?

        delete fd;

        queue(proc->threads[0]);
        
        return true;
    }

    void queue(thread* thread_to_queue) {
        Lock::acquire(&sched_lock);
        
        waiting_queue.push(thread_to_queue);

        Lock::release(&sched_lock);
    }

    extern "C" void reschedule(context* regs) {
        cpu* current_core = Cpu::local_core();
        
        Lock::acquire(&sched_lock);
        
        auto queue_obj = waiting_queue.pop().value();
        thread* scheduled_thread = (queue_obj != nullptr) ? *queue_obj : nullptr;
       
        if (scheduled_thread == nullptr) {
            if (current_core->running_thread != nullptr) {
                scheduled_thread = current_core->running_thread;
            } else {
                Lock::release(&sched_lock);
                Apic::localApic->eoi();
                return;
            }
        }   
        
        if (current_core->running_thread != nullptr && current_core->running_thread != scheduled_thread) {
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

void syscall_fork(context* regs) {
    cpu* current_core = Cpu::local_core();

    Sched::process* parent = current_core->running_process;
    Sched::process* child = new Sched::process;

    Lock::acquire(&sched_lock);
    process_list.push_back(child);
    Lock::release(&sched_lock);
    allocator->dump_heap();

    child->pid = Sched::get_new_pid();

    child->status = Sched::Status::Waiting;
    child->process_directory = parent->process_directory;
    
    child->pagemap = parent->pagemap->duplicate();
    
    for (size_t i = 0; i < parent->fd_list.size(); i++) {
        auto fd = new Vfs::file_description;
        memcpy(fd, parent->fd_list[i], sizeof(Vfs::file_description));

        child->fd_list.push_back(fd);
    }
    Sched::thread* main_thread = new Sched::thread;
    log("thread addr: %x\n", (uint64_t)main_thread);
    allocator->dump_heap();
    main_thread->regs = new context;

    int64_t new_tid = Sched::get_new_tid();
    if (new_tid == -1) {
        regs->rax = -1;
    }

    main_thread->tid = new_tid;
    main_thread->status = Sched::Status::Waiting;
    main_thread->parent_process = child;

    memcpy(main_thread->regs, regs, sizeof(context));
    main_thread->regs->rax = 0;

    child->threads.push_back(main_thread);
    allocator->dump_heap();
    Sched::queue(main_thread);

    regs->rax = child->pid;
}

// void syscall_execve(context* regs) {
//     cpu* current = Cpu::local_core();

    
// }