.global loadGdt
.type loadGdt, @function

loadGdt:
    lgdt (%rdi)
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %ss
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %es
    pop %rdi
    pushq $0x08
    push %rdi //return address
    retfq

.global loadIdt
.type loadIdt, @function

loadIdt:
    lidt (%rdi)
    ret
