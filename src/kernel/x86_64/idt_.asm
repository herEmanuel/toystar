%macro prologue 0

    mov rdi, rsp

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

%endmacro

%macro epilogue 0

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

    iretq

%endmacro

global _isr_div_by_zero
extern isr_div_by_zero
_isr_div_by_zero:
    prologue
    call isr_div_by_zero
    epilogue

global _isr_breakpoint
extern isr_breakpoint
_isr_breakpoint:
    prologue
    call isr_breakpoint
    epilogue

global _isr_double_fault
extern isr_double_fault
_isr_double_fault:
    prologue
    call isr_double_fault
    epilogue

global _isr_general_protection
extern isr_general_protection
_isr_general_protection:
    prologue
    call isr_general_protection
    epilogue

global _isr_page_fault
extern isr_page_fault
_isr_page_fault:
    prologue
    call isr_page_fault
    epilogue

global _isr_keyboard
extern isr_keyboard
_isr_keyboard:
    prologue
    call isr_keyboard
    epilogue
