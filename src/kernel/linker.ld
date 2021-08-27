OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(_start)

PHDRS
{
    null    PT_NULL    FLAGS(0) ;                   /* Null segment */
    rodata  PT_LOAD    FLAGS((1 << 2)) ;            /* Read only */
    text    PT_LOAD    FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
    data    PT_LOAD    FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
    dynamic PT_DYNAMIC FLAGS((1 << 1) | (1 << 2)) ; /* Dynamic segment needed for PIE */
}

SECTIONS
{
    . = 0xffffffff80200000;

    .stivale2hdr ALIGN(4K) : {
        KEEP(*(.stivale2hdr))
    } :rodata

    .rodata ALIGN(4K) : {
        start_ctors = .;
        *(SORT(.ctors*))  /* Note the "SORT" */
        end_ctors = .;

        start_dtors = .;
        *(SORT(.dtors*))
        end_dtors = .;

        *(.rodata*)
    } :rodata

    .text ALIGN(4K) : {
        *(.text*)
    } :text

    .data ALIGN(4K) : {
        *(.data*)
    } :data

    .dynamic ALIGN(4K) : {
        *(.dynamic)
    } :data :dynamic

    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss*)
    } :data
}
