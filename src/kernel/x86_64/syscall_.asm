global syscall_entry
extern syscall_main

syscall_entry:
    swapgs 

    mov [gs:0], rsp  ; save user stack
    mov rsp, [gs:8] ; load kernel stack

    sti

    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax

    cld

    mov rdi, rsp
    call syscall_main

    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    cli

    mov rsp, [gs:0] ; restore user stack

    swapgs

    iretq
