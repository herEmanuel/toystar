#include "acpi.hpp"
#include <boot/stivale2.hpp>
#include <memory/vmm.hpp>
#include <video.hpp>
#include <strings.hpp>
#include <stddef.h>

static rsdt* rsdt = nullptr;

namespace Acpi {

    void init(stivale2_struct_tag_rsdp* rsdpTag) {
        rsdp* rsdpStruct = reinterpret_cast<struct rsdp*>(rsdpTag->rsdp);

        kprint("ACPI version: %d\n", rsdpStruct->revision);
        kprint("Signature: %s\n", rsdpStruct->signature);

        rsdt = reinterpret_cast<struct rsdt*>(rsdpStruct->rsdt_addr + PHYSICAL_BASE_ADDRESS);
        kprint("RSDT: %x\n", (size_t)rsdt);
    }

    void* findTable(const char* signature) {
        size_t tablesNum = (rsdt->header.length - sizeof(sdt))/4;

        for (size_t i = 0; i < tablesNum; i++) {
            sdt* currentTable = reinterpret_cast<sdt*>(rsdt->tables[i] + PHYSICAL_BASE_ADDRESS);

            if (strcmp(currentTable->signature, signature, 4)) {
                kprint("Found the table %s\n", currentTable->signature);
                return reinterpret_cast<void*>(currentTable);
            }
        }

        return nullptr;
    }

}