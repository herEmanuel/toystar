#ifndef MADT_H
#define MADT_H

#include "acpi.hpp"
#include <stdint.h>

enum Types {
    LAPIC,
    IOAPIC,
    IOAPIC_ISO,
    IOAPIC_NMI,
    LAPIC_AO = 5
};

struct madt_table {
    sdt header;
    uint32_t lapic_addr;
    uint32_t flags;
    uint8_t madt_entries[]; //variable length entries
} __attribute__((packed));

struct madt_header {
    uint8_t type;
    uint8_t length;
} __attribute__((packed));

struct madt_lapic {
    madt_header header;
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__((packed));

struct madt_ioapic {
    madt_header header;
    uint8_t ioapic_id;
    uint8_t reserved;
    uint32_t ioapic_addr;
    uint32_t gsib;
} __attribute__((packed));

struct madt_iso {
    madt_header header;
    uint8_t bus;
    uint8_t irq;
    uint32_t gsi;
    uint16_t flags;
};

struct madt_addrOverride {
    madt_header header;
    uint16_t reserved;
    uint64_t addr;
};

#endif