#include "scheduler.hpp"

#include <stdint.h>
#include <stddef.h>
#include <vector.hpp>
#include <lock.hpp>
#include <x86_64/cpu.hpp>
#include <memory/vmm.hpp>
#include <memory/pmm.hpp>
#include <memory/heap.hpp>

Lock::lock_t sched_lock;

toys::vector<process*> process_list;

process* idle_process = nullptr;

//TODO: non-sequential pids/tids?

void idle_process_func() {
    Cpu::halt();
}

void scheduler_init() {
    idle_process = new process;
    idle_process->status = Status::Waiting;
    idle_process->pid = 0;
    idle_process->waiting_time = 0;
    process_list.push_back(idle_process);

    thread* idle_thread = new thread;
    idle_thread->tid = 0;
    idle_thread->status = Status::Waiting;
    idle_thread->waiting_time = 0;
    idle_thread->parent_pid = 0;
    idle_thread->regs->rip = (uint64_t)&idle_process_func;

    idle_process->threads.push_back(idle_thread);
    reschedule(idle_thread->regs);
}

process* create_process(uint64_t rip, VMM::Pagemap* pagemap) {
    process* newProcess = new process;

    newProcess->status = Status::Waiting;
    newProcess->pid = process_list.size();
    newProcess->pagemap = pagemap;
    newProcess->waiting_time = 0;

    thread* mainThread = new thread;

    mainThread->tid = 0; //note: nonono
    mainThread->status = Status::Waiting;
    mainThread->waiting_time = 0;
    mainThread->kernel_stack = reinterpret_cast<uint64_t>(PMM::alloc(1) + VMM::PAGE_SIZE + VMM::PHYSICAL_BASE_ADDRESS);
    // mainThread->user_stack = VMM::vmm->mapRange()
    mainThread->regs->rip = rip;
    mainThread->regs->rsp = mainThread->user_stack;
}

//TODO: how can 2 threads of the same process be running at the same time?

extern "C" void reschedule(context* regs) {

    size_t waiting_amount = 0;
    size_t proc_pid;
    size_t thread_id;

    Lock::acquire(sched_lock);

    for (size_t i = 0; i < process_list.size(); i++) {
        if (process_list[i]->status == Status::Running) 
            continue;

        if (process_list[i]->waiting_time > waiting_amount) {
            waiting_amount = process_list[i]->waiting_time;
            proc_pid = process_list[i]->pid;

            process_list[i]->waiting_time++;
        }

    }

    process* proc = process_list[proc_pid];
    waiting_amount = 0;

    for (size_t i = 0; i < proc->threads.size(); i++) {
        if (proc->threads[i]->waiting_time > waiting_amount) {
            waiting_amount = proc->threads[i]->waiting_time;
            thread_id = proc->threads[i]->tid;

            proc->threads[i]->waiting_time++;
        }
    }

    thread* scheduled_thread = proc->threads[thread_id];

    proc->status = Status::Running;
    scheduled_thread->status = Status::Running;

    //TODO: save current state

    cpu* current_core = Cpu::local_core();

    process* previous_proc = process_list[current_core->pid];

    previous_proc->status = Status::Waiting;
    previous_proc->threads[current_core->tid]->status = Status::Waiting;
    previous_proc->threads[current_core->tid]->regs = regs;

    Lock::release(sched_lock);

    current_core->tid = scheduled_thread->tid;
    current_core->pid = proc->pid;
    current_core->kernel_stack = scheduled_thread->kernel_stack;
    current_core->user_stack = scheduled_thread->user_stack;

    context_switch(scheduled_thread->regs);
}