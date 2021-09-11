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

//TODO: non-sequential pids/tids?

Sched::process* init_process = nullptr;

void init_proc() {
    // asm volatile("xor %rax, %rax");
    // asm volatile("int $0x80");
    kprint("Main process started!\n");
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

    process* create_process(uint64_t rip, uint64_t cs, VMM::vmm* pagemap) {
        process* newProcess = new process;

        newProcess->status = Status::Waiting;
        newProcess->pid = process_list.size();
        newProcess->pagemap = pagemap;

        thread* mainThread = new thread;

        mainThread->tid = thread_list.size(); 
        mainThread->parent_pid = newProcess->pid;
        mainThread->status = Status::Waiting;
        mainThread->waiting_time = 0;

        memset(mainThread->regs, 0, sizeof(context));

        uint64_t stack = (uint64_t) PMM::alloc(USER_STACK_SIZE / PAGE_SIZE);
        
        if (cs & 0x3) {
            //userland process
            mainThread->kernel_stack = reinterpret_cast<uint64_t>(PMM::alloc(1) + PAGE_SIZE + PHYSICAL_BASE_ADDRESS);

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
        process_list.push_back(newProcess);
        Lock::release(&sched_lock);
        
        return newProcess;  
    }

    void queue(thread* thread_to_queue) {
        Lock::acquire(&sched_lock);

        if (thread_to_queue->tid < thread_list.size()) {
            return;
        }
        
        thread_list.push_back(thread_to_queue);

        Lock::release(&sched_lock);
    }
    
    extern "C" void reschedule(context* regs) {
        size_t waiting_amount = 0;
        int thread_id = -1;

        if (regs->cs & 0x3) {
            Cpu::swapgs();
        }
 
        cpu* current_core = Cpu::local_core();

        Lock::acquire(&sched_lock);
        
        for (size_t i = 0; i < thread_list.size(); i++) {
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
                if (regs->cs & 0x3) {
                    Cpu::swapgs();
                }   
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
        current_core->kernel_stack = scheduled_thread->kernel_stack;
        current_core->user_stack = scheduled_thread->user_stack;
        current_core->pagemap = parent_process->pagemap;
        current_core->working_dir = parent_process->process_directory;
        
        parent_process->pagemap->switch_pagemap();

        if (scheduled_thread->regs->cs & 0x3) {
            Cpu::swapgs();
        }
        
        Apic::localApic->eoi();
        
        context_switch(scheduled_thread->regs);
    }
}