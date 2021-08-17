#ifndef ACPI_H
#define ACPI_H

#include <boot/stivale2.hpp>
#include <stdint.h>

struct sdt {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t creatorId;
    uint32_t creatorRevision;
} __attribute__((packed));

struct rsdp {
    char signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint8_t revision;
    uint32_t rsdt_addr;

    //version 2.0 ->

    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t extendedChecksum;
    uint8_t reserved[3];
} __attribute__((packed));

struct rsdt {
    sdt header;
    uint32_t tables[];
} __attribute__((packed));

namespace Acpi {

    void init(stivale2_struct_tag_rsdp* rsdpTag);
    void* findTable(const char* signature);

}

#endif