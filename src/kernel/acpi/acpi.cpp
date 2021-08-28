#include "acpi.hpp"
#include <boot/stivale2.hpp>
#include <memory/vmm.hpp>
#include <strings.hpp>
#include <stddef.h>

static rsdt* rsdt = nullptr;

namespace Acpi {

    //TODO: support to xsdt
    void init(stivale2_struct_tag_rsdp* rsdpTag) {
        rsdp* rsdpStruct = reinterpret_cast<struct rsdp*>(rsdpTag->rsdp);

        rsdt = reinterpret_cast<struct rsdt*>(rsdpStruct->rsdt_addr + PHYSICAL_BASE_ADDRESS);
    }

    void* findTable(const char* signature) {
        size_t tablesNum = (rsdt->header.length - sizeof(sdt))/4;

        for (size_t i = 0; i < tablesNum; i++) {
            sdt* currentTable = reinterpret_cast<sdt*>(rsdt->tables[i] + PHYSICAL_BASE_ADDRESS);

            if (strcmp(currentTable->signature, signature, 4)) {
                return reinterpret_cast<void*>(currentTable);
            }
        }

        return nullptr;
    }

}