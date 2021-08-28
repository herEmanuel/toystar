global loadGdt

loadGdt:
    lgdt [rdi]
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    mov es, ax
    pop rdi
    push 0x08
    push rdi ; return address
    retfq

