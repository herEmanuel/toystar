#include "scheduler.hpp"

#include <stdint.h>
#include <stddef.h>
#include <vector.hpp>
#include <lock.hpp>
#include <x86_64/cpu.hpp>
#include <memory/vmm.hpp>
#include <memory/pmm.hpp>
#include <memory/heap.hpp>

Lock::lock_t sched_lock = 0;

toys::vector<process*> process_list;

process* idle_process = nullptr;

//TODO: non-sequential pids/tids?

void idle_process_func() {
    kprint("Idle proces!!!!!\n");
    Cpu::halt();
}

void scheduler_init() {
    VMM::vmm* proc_vmm = new VMM::vmm(true);

    proc_vmm->mapRangeRaw(0, 0, 1048576, 0x7);

    process* idleProcess = create_process((uint64_t)&idle_process_func, 0x8, proc_vmm);
    
    process_list.push_back(idleProcess);
    reschedule(idleProcess->threads[0]->regs);
}

process* create_process(uint64_t rip, uint64_t cs, VMM::vmm* pagemap) {
    process* newProcess = new process;

    newProcess->status = Status::Waiting;
    newProcess->pid = process_list.size();
    newProcess->pagemap = pagemap;
    newProcess->waiting_time = 0;

    thread* mainThread = new thread;

    uint64_t stack = (uint64_t) PMM::alloc(USER_STACK_SIZE / PAGE_SIZE);

    mainThread->tid = 0; //note: nonono
    mainThread->status = Status::Waiting;
    mainThread->waiting_time = 0;
    mainThread->kernel_stack = reinterpret_cast<uint64_t>(PMM::alloc(1) + PAGE_SIZE + PHYSICAL_BASE_ADDRESS);

    pagemap->mapRange(USER_STACK_TOP, stack + USER_STACK_SIZE, 
                        USER_STACK_SIZE / PAGE_SIZE, 0x7, 0);
    
    mainThread->user_stack = USER_STACK_TOP;
    mainThread->regs->rip = rip;
    mainThread->regs->rsp = mainThread->user_stack;
    mainThread->regs->rflags = 0x202;

    mainThread->regs->cs = cs;

    if (cs == 0x8) {
        mainThread->regs->ss = 0x10;
    } else {
        mainThread->regs->ss = 0x20;
    }

    newProcess->threads.push_back(mainThread);

    return newProcess;
}

//TODO: how can 2 threads of the same process be running at the same time?

extern "C" void reschedule(context* regs) {

    size_t waiting_amount = 0;
    int proc_pid = -1;
    int thread_id = -1;
    
    Lock::acquire(&sched_lock);

    for (size_t i = 0; i < process_list.size(); i++) {
        if (process_list[i]->status == Status::Running) 
            continue;

        if (process_list[i]->waiting_time >= waiting_amount) {
            waiting_amount = process_list[i]->waiting_time;
            proc_pid = process_list[i]->pid;

            process_list[i]->waiting_time++;
        }

    }

    if (proc_pid == -1) {
        kprint("yeah theres nothing there\n");
        Cpu::halt();
    }

    process* proc = process_list[proc_pid];
    waiting_amount = 0;

    for (size_t i = 0; i < proc->threads.size(); i++) {
        if (proc->threads[i]->waiting_time >= waiting_amount) {
            waiting_amount = proc->threads[i]->waiting_time;
            thread_id = proc->threads[i]->tid;

            proc->threads[i]->waiting_time++;
        }
    }

    if (thread_id == -1) {
        kprint("no threads bruh\n");
        Cpu::halt();
    }

    thread* scheduled_thread = proc->threads[thread_id];

    proc->status = Status::Running;
    scheduled_thread->status = Status::Running;

    //TODO: save current state
    //TODO: theres nothing at gs now bruh
    cpu* current_core = Cpu::local_core();

    process* previous_proc = process_list[current_core->pid];

    previous_proc->status = Status::Waiting;
    previous_proc->threads[current_core->tid]->status = Status::Waiting;
    previous_proc->threads[current_core->tid]->regs = regs;

    Lock::release(&sched_lock);

    current_core->tid = scheduled_thread->tid;
    current_core->pid = proc->pid;
    current_core->kernel_stack = scheduled_thread->kernel_stack;
    current_core->user_stack = scheduled_thread->user_stack;

    proc->pagemap->switchPagemap();

    context_switch(scheduled_thread->regs);
}