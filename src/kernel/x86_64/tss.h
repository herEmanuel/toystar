#ifndef TSS_H
#define TSS_H

#include <stdint.h>

//TODO: make it work 

struct TSS {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved2;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved3;
    uint32_t iobm; //TODO: still don't know exactly what this is
} __attribute__((packed));

#endif