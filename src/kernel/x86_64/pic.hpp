#ifndef PIC_H
#define PIC_H

#include <stddef.h>
#include <stdint.h>

#define MASTER_COMMAND 0x20
#define MASTER_DATA 0x21
#define SLAVE_COMMAND 0xA0
#define SLAVE_DATA 0xA1

void remap_pic(size_t vecOffset1, size_t vecOffset2);

#endif