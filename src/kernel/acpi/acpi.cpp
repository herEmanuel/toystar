#include "acpi.hpp"
#include <boot/stivale2.hpp>
#include <memory/vmm.hpp>
#include <strings.hpp>
#include <stddef.h>

static rsdt* rsdt = nullptr;

namespace Acpi {

    //TODO: support to xsdt
    void init(stivale2_struct_tag_rsdp* rsdp_tag) {
        rsdp* rsdp_struct = reinterpret_cast<struct rsdp*>(rsdp_tag->rsdp);

        rsdt = reinterpret_cast<struct rsdt*>(rsdp_struct->rsdt_addr + PHYSICAL_BASE_ADDRESS);
    }

    void* find_table(const char* signature) {
        size_t tablesNum = (rsdt->header.length - sizeof(sdt))/4;

        for (size_t i = 0; i < tablesNum; i++) {
            sdt* current_table = reinterpret_cast<sdt*>(rsdt->tables[i] + PHYSICAL_BASE_ADDRESS);

            if (strncmp(current_table->signature, signature, 4)) {
                return reinterpret_cast<void*>(current_table);
            }
        }

        return nullptr;
    }

}