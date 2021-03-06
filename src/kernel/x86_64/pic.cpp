#include "pic.hpp"
#include <kernel/io.hpp>

void remap_pic(size_t vecOffset1, size_t vecOffset2) {
    outb(MASTER_COMMAND, 0x11);
    outb(SLAVE_COMMAND, 0x11);

    outb(MASTER_DATA, vecOffset1);
    outb(SLAVE_DATA, vecOffset2);

    outb(MASTER_DATA, 4); //master's irq2
    outb(SLAVE_DATA, 2);  //slave's irq9

    outb(MASTER_DATA, 0x01);
    outb(SLAVE_DATA, 0x01);

    //sets the mask for each PIC
    outb(MASTER_DATA, 0xFF); //0xFF disables all hardware interrupts
    outb(SLAVE_DATA, 0xFF);
}