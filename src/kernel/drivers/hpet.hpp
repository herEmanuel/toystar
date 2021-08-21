#ifndef HPET_H
#define HPET_H

#include <acpi/acpi.hpp>
#include <stdint.h>

struct hpet_table {
    sdt header;
    uint8_t revision_id;
    uint8_t details;
    uint16_t pci_id;
    uint8_t address_space_id;
    uint8_t register_width;
    uint8_t register_offset;
    uint8_t reserved;
    uint64_t address;
    uint8_t hpet_number;
    uint16_t minimum_ticks;
    uint8_t page_protection;
} __attribute__((packed));

//memory mapped registers
struct hpetMem {
    uint64_t general_capabilities;
    uint64_t unused0;
    uint64_t general_configuration;
    uint64_t unused1;
    uint64_t interrupt_status;
    uint64_t unused2[25];
    uint64_t main_counter_val;
};

namespace Hpet {
    void init();
    void sleep(uint64_t ms);
}

#endif